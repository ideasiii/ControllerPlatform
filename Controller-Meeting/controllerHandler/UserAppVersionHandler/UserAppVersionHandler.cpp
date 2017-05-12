#include "UserAppVersionHandler.h"

#include <errno.h>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include "HiddenUtility.hpp"
#include "LogHandler.h"

#define INOTIFY_EVENT_SIZE  (sizeof (struct inotify_event))
#define INOTIFY_BUF_LEN     (1024 * (INOTIFY_EVENT_SIZE + 16))

void *threadStartRoutine_UserAppVersionHandler_runWatcher(void *argv)
{
	_log("[UserAppVersionHandler] Do runWatcher() in a thread");

	watcherThreadId = getThreadID();
	auto uadlh = reinterpret_cast<UserAppVersionHandler*>(argv);
	uadlh->runWatcher();			
	
	return 0;
}

UserAppVersionHandler::UserAppVersionHandler(std::string watchDirectory, int inotifyMask) : 
	watchDir(watchDirectory), lastUpdated(-1),
	stopSignalPipeFd{ -1, -1 }, inotifyEventMask(inotifyMask)
{
}

UserAppVersionHandler::~UserAppVersionHandler()
{
	stop();
}

void UserAppVersionHandler::start()
{
	int ret = pipe(stopSignalPipeFd);
	if (ret < 0)
	{
		_log("[UserAppVersionHandler] start() cannot create pipe");	
		return;
	}

	createThread(threadStartRoutine_UserAppVersionHandler_runWatcher, 
		this, "UserAppVersionHandler Watcher Thread");
}

void UserAppVersionHandler::stop()
{
	if (0 >= watcherThreadId)
	{
		return;
	}

	pthread_detach(pthread_self());

	_log("[UserAppVersionHandler] stop() stopSignalPipeFd[1] = %d", stopSignalPipeFd[1]);
	if (stopSignalPipeFd[1] > 0)
	{
		char whatever = 'a';
		_log("[UserAppVersionHandler] stop() write to pipe");
		write(stopSignalPipeFd[1], &whatever, 1);
		_log("[UserAppVersionHandler] stop() write to pipe ok");
		close(stopSignalPipeFd[1]);
		stopSignalPipeFd[1] = -1;
	}

	/*_log("[UserAppVersionHandler::stop()] threadCancel(watcherThreadId)");
	threadCancel(watcherThreadId);
	_log("[UserAppVersionHandler::stop()] threadJoin(watcherThreadId)");
	threadJoin(watcherThreadId);*/
	watcherThreadId = 0;
}

void UserAppVersionHandler::runWatcher()
{
	if (stopSignalPipeFd[0] < 0 || stopSignalPipeFd[1] < 0)
	{
		_log("[UserAppVersionHandler] runWatcher() broken pipe??");
		return;
	}

	_log("[UserAppVersionHandler] Initializing members once before going into watch loop");
	reload();

	bool doLoop = true;
	while(doLoop)
	{
		int inotifyFd, inotifyWd;
		do
		{
			int ret = HiddenUtility::initInotifyWithOneWatchDirectory(
				&inotifyFd, &inotifyWd, watchDir.c_str(), inotifyMask);
			if (ret == 0)
			{
				break;
			}	
			
			lastUpdated = -1;
			sleep(10);
		} while(true);
		
		int maxFd = (stopSignalPipeFd[0] > inotifyFd ? stopSignalPipeFd[0] : inotifyFd) + 1;
		fd_set readFdSet;
		FD_ZERO(&readFdSet);

		while(true)
		{	
			FD_SET(inotifyFd, &readFdSet);
			FD_SET(stopSignalPipeFd[0], &readFdSet);
			
			_log("[UserAppVersionHandler] Watching changes in %s", apkDir.c_str());
			
			int ret = select(maxFd, &readFdSet, NULL, NULL, NULL);
			if (ret < 0) 
			{
				if (errno != EINTR)
				{
					_log("[UserAppVersionHandler] select() returned with EINTR, try again");
					continue;
				}

				_log("[UserAppVersionHandler] select() failed: %s", strerror(errno));
				break;
			}

			if (FD_ISSET(stopSignalPipeFd[0], &readFdSet))
			{
				_log("[UserAppVersionHandler] Received signal from pipe, quit runWatcher() loop");
				doLoop = false;

				close(stopSignalPipeFd[0]);
				stopSignalPipeFd[0] = -1;

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
				struct inotify_event *event = (struct inotify_event *) &buf[i];
				if (event->len) 
				{
					bool shouldContinue = onInotifyEvent(event);
					if (!shouldContinue)
					{
						break;
					}
				}

				i += INOTIFY_EVENT_SIZE + event->len;
			}
		}

		inotify_rm_watch(inotifyFd, inotifyWd);
		close(inotifyFd);
		
		if (doLoop)
		{
			sleep(10);
		}
	}
}

void UserAppVersionHandler::onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen,
	const void* pData)
{
	_log("[UserAppVersionHandler] I ignore everything passed to onReceiveMessage()");
}
