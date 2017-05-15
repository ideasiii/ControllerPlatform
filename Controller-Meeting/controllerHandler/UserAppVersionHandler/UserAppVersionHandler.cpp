#include "UserAppVersionHandler.h"

#include <errno.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "../HiddenUtility.hpp"
#include "LogHandler.h"

#define INOTIFY_EVENT_SIZE  (sizeof (struct inotify_event))
#define INOTIFY_BUF_LEN     (1024 * (INOTIFY_EVENT_SIZE + 16))

void *threadStartRoutine_UserAppVersionHandler_runWatcher(void *argv)
{

	_log("[UserAppVersionHandler] threadStartRoutine_UserAppVersionHandler_runWatcher() step in");

	auto uadlh = reinterpret_cast<UserAppVersionHandler*>(argv); 
	uadlh->watcherThreadId = pthread_self();
	uadlh->doLoop = true;
	uadlh->runWatcher();			
	
	_log("[UserAppVersionHandler] threadStartRoutine_UserAppVersionHandler_runWatcher() step out");
	return 0;
}

UserAppVersionHandler::UserAppVersionHandler(std::string watchDirectory, int inotifyMask) : 
	watchDir(watchDirectory), lastUpdated(-1), inotifyEventMask(inotifyMask), doLoop(false)
{
}

UserAppVersionHandler::~UserAppVersionHandler()
{
	stop();
}

void UserAppVersionHandler::start()
{
	createThread(threadStartRoutine_UserAppVersionHandler_runWatcher, 
		this, "UserAppVersionHandler Watcher Thread");
}

void UserAppVersionHandler::stop()
{
	if (0 == watcherThreadId)
	{
		_log("[UserAppVersionHandler] stop() 0 == watcherThreadId");
		return;
	}

	doLoop = false;

	//pthread_detach(watcherThreadId);
	threadJoin(watcherThreadId);
	watcherThreadId = 0;
}

void UserAppVersionHandler::runWatcher()
{
	_log("[UserAppVersionHandler] Initializing members once before going into watch loop");
	reload();

	while(doLoop)
	{
		int inotifyFd, inotifyWd;
		do
		{
			int ret = HiddenUtility::initInotifyWithOneWatchDirectory(
				&inotifyFd, &inotifyWd, watchDir.c_str(), inotifyEventMask);
			if (ret == 0)
			{
				_log("[UserAppVersionHandler] Watch created, break creating loop");
				break;
			}	
			
			lastUpdated = -1;
			sleep(10);
		} while(true);
		
		int maxFd = inotifyFd + 1;
		
				
		_log("[UserAppVersionHandler] Watching changes in %s", watchDir.c_str());
		
		while(true)
		{
			fd_set readFdSet;
			FD_ZERO(&readFdSet);
			FD_SET(inotifyFd, &readFdSet);
			
			// On Linux, select() modifies timeout to reflect the amount of time not slept
			struct timeval timeout;
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			//_log("[UserAppVersionHandler] select() step in");
			int ret = select(maxFd, &readFdSet, NULL, NULL, &timeout);
			//_log("[UserAppVersionHandler] select() stepped out, ret = %d", ret);

			if (ret == 0)
			{
				// timeout
				if (!doLoop)
				{
					_log("[UserAppVersionHandler] doLoop = false, break loop", ret);
					break;
				}
				else
				{
					continue;
				}
			}
			else if (ret < 0)
			{
				if (errno != EINTR)
				{
					_log("[UserAppVersionHandler] select() returned with EINTR, try again");
					continue;
				}

				_log("[UserAppVersionHandler] select() failed: %s", strerror(errno));
				break;
			}
			
			uint8_t buf[INOTIFY_BUF_LEN];
			size_t len = read(inotifyFd, buf, INOTIFY_BUF_LEN);
			
			if (len < 1)
			{
				_log("[UserAppVersionHandler] read() failed: %s", strerror(errno));
				break;
			}

			_log("[UserAppVersionHandler] Detected dir change, len = %d", len);
			
			int i = 0;
			while (i < len) 
			{
				// TODO MOVED_FROM and MOVE_TO and IGNORE handling
				// when dir is moved from original tree
				struct inotify_event *event = (struct inotify_event *) &buf[i];
				if (event->len && (event->mask & inotifyEventMask)) 
				{
					if (!onInotifyEvent(event))
					{
						break;
					}
				}

				i += INOTIFY_EVENT_SIZE + event->len;
			}
		}

		inotify_rm_watch(inotifyFd, inotifyWd);
		close(inotifyFd);
		inotifyFd = -1;
		
		if (doLoop)
		{
			_log("[UserAppVersionHandler] Sleep before doing next watch loop");
			sleep(10);
		}	
	}

	_log("[UserAppVersionHandler] Exit runWatcher()");
}

void UserAppVersionHandler::onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen,
	const void* pData)
{
	_log("[UserAppVersionHandler] I ignore everything passed to onReceiveMessage()");
}
