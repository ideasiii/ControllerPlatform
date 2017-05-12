#include "UserAppVersionHandler.h"

#include <dirent.h>
#include <errno.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>
#include "AndroidPackageInfoQuierer.hpp"
#include "HiddenUtility.hpp"
#include "LogHandler.h"
#include "JSONObject.h"

#define INOTIFY_EVENT_SIZE  (sizeof (struct inotify_event))
#define INOTIFY_BUF_LEN     (1024 * (INOTIFY_EVENT_SIZE + 16))

void *threadConfigChangeWatcher(void *argv)
{
	auto uadlh = reinterpret_cast<UserAppVersionHandler*>(argv);
	uadlh->runConfigChangeWatcher();
	return 0;
}

void *threadNewApkWatcher(void *argv)
{
	auto uadlh = reinterpret_cast<UserAppVersionHandler*>(argv);
	uadlh->runNewApkWatcher();
	return 0;
}

UserAppVersionHandler::UserAppVersionHandler(std::string dir, std::string name) :
		useConfigWatcherMethod(true), configDir(dir), configName(name), 
		useApkScanningMethod(false), lastUpdated(-1), stopSignalPipeFd{ -1, -1 }
{
}

UserAppVersionHandler::UserAppVersionHandler
	(AndroidPackageInfoQuierer *q, std::string pkgName, std::string dir, std::string dlLinkBase) :
		useConfigWatcherMethod(false), useApkScanningMethod(true), apkDir(dir), 
		apkQuierer(q), downloadLinkBasePath(dlLinkBase), packageName(pkgName),
		lastUpdated(-1), stopSignalPipeFd{ -1, -1 }
{
}

UserAppVersionHandler:: ~UserAppVersionHandler()
{
}

void UserAppVersionHandler::start()
{
	int ret = pipe(stopSignalPipeFd);
	if (ret < 0)
	{
		_log("[UserAppVersionHandler] start() cannot create pipe");	
		return;
	}

	if (useApkScanningMethod)
	{
		_log("[UserAppVersionHandler] start() with apk watching");
		startNewApkWatcher();
	}
	else if (useConfigWatcherMethod)
	{
		_log("[UserAppVersionHandler] start() with config watching");
		startConfigChangeWatcher();
	}
	else
	{
		_log("[UserAppVersionHandler] start() cannot start??");
		close(stopSignalPipeFd[0]);
		close(stopSignalPipeFd[1]);
	}
}

void UserAppVersionHandler::startConfigChangeWatcher()
{
	createThread(threadConfigChangeWatcher, this,
		"UserAppVersionHandler Config Watcher Thread");
}

void UserAppVersionHandler::startNewApkWatcher()
{
	createThread(threadNewApkWatcher, this,
		"UserAppVersionHandler New APK Watcher Thread");
}

void UserAppVersionHandler::stop()
{
	if (stopSignalPipeFd > 0)
	{
		char whatever = 'a';
		write(stopSignalPipeFd[1], &whatever, 1);
		close(stopSignalPipeFd[1]);
	}

	if (0 < watcherThreadId)
	{
		//threadCancel(watcherThreadId);
		threadJoin(watcherThreadId);
		watcherThreadId = 0;
	}
}

void UserAppVersionHandler::runNewApkWatcher()
{
	this->watcherThreadId = getThreadID();
	bool doit = true;
	while(doit)
	{
		int inotifyFd, inotifyWd;
		do
		{
			_log("[UserAppVersionHandler] Adding watch to %s", apkDir.c_str());
			int ret = HiddenUtility::initInotifyWithOneWatchDirectory(
				&inotifyFd, &inotifyWd, apkDir.c_str(), IN_CLOSE_WRITE);
			if (ret == 0)
			{
				break;
			}	
			
			lastUpdated = -1;
			sleep(10);
		} while(true);
		

		_log("[UserAppVersionHandler] Initializing members by finding newest apk file");
		reloadNewestApkInfo();
		
		fd_set readFdSet;
		FD_ZERO(&readFdSet);
		FD_SET(inotifyFd, &readFdSet);
		if (stopSignalPipeFd[0] > 0)
		{
			FD_SET(stopSignalPipeFd[0], &readFdSet);
		}
		int maxFd = (stopSignalPipeFd[0] > inotifyFd ? stopSignalPipeFd[0] : inotifyFd) + 1;

		while(true)
		{	
			_log("[UserAppVersionHandler] Watching changes in %s", apkDir.c_str());
			
			int ret = select(maxFd, &readFdSet, NULL, NULL, NULL);
			if ((ret < 0) && (errno!=EINTR)) 
			{
				_log("[UserAppVersionHandler] select() failed: %s", strerror(errno));
				break;
			}

			if (FD_ISSET(stopSignalPipeFd[0], &readFdSet))
			{
				doit = false;
				break;
			}

			uint8_t buf[INOTIFY_BUF_LEN];
			size_t len = read(inotifyFd, buf, INOTIFY_BUF_LEN);
			
			if (len < 1)
			{
				_log("[UserAppVersionHandler] read() failed: %s", strerror(errno));
				break;
			}

			_log("[UserAppVersionHandler] Detected apk dir change, len = %d", len);
			
			int i = 0;
			while (i < len) 
			{
				struct inotify_event *event = (struct inotify_event *) &buf[i];
				if (event->len) 
				{
					if (event->mask & IN_CLOSE_WRITE
						&& HiddenUtility::strEndsWith((char*)event->name, ".apk")
						&& HiddenUtility::unixTimeMilli() - lastUpdated > 1000)
					{
						_log("[UserAppVersionHandler] APK created in apk dir, reloading");
						reloadNewestApkInfo();
						break;
					}
				}

				i += INOTIFY_EVENT_SIZE + event->len;
			}
		}

		inotify_rm_watch(inotifyFd, inotifyWd);
		close(inotifyFd);
	}

	if (stopSignalPipeFd[0] > 0)
	{
		close(stopSignalPipeFd[0]);
	}
}

void UserAppVersionHandler::runConfigChangeWatcher()
{
	this->watcherThreadId = getThreadID();

	while(true)
	{
		int inotifyFd, inotifyWd;
		do
		{
			_log("[UserAppVersionHandler] Adding watch to %s", configDir.c_str());
			int ret = HiddenUtility::initInotifyWithOneWatchDirectory(
				&inotifyFd, &inotifyWd, configDir.c_str(), IN_CLOSE_WRITE);
			if (ret == 0)
			{
				break;
			}	
			
			lastUpdated = -1;
			sleep(10);
		} while(true);
		

		_log("[UserAppVersionHandler] Initializing members using config file");
		reloadConfig();
		
		while(true)
		{	
			_log("[UserAppVersionHandler] Watching changes in %s", configDir.c_str());
			uint8_t buf[INOTIFY_BUF_LEN];
			size_t len = read(inotifyFd, buf, INOTIFY_BUF_LEN);
			
			if (len < 1)
			{
				_log("[UserAppVersionHandler] read() failed: %s", strerror(errno));
				break;
			}

			_log("[UserAppVersionHandler] Detected config dir change, len = %d", len);
			
			int i = 0;
			while (i < len) 
			{
				struct inotify_event *event = (struct inotify_event *) &buf[i];
				if (event->len) 
				{
					if (event->mask & IN_CLOSE_WRITE
						&& strcmp(configName.c_str(), (char*)event->name) == 0
						&& HiddenUtility::unixTimeMilli() - lastUpdated > 1000)
					{
						_log("[UserAppVersionHandler] Config changed, reloading");
						reloadConfig();
						break;
					}
				}

				i += INOTIFY_EVENT_SIZE + event->len;
			}
		}

		inotify_rm_watch(inotifyFd, inotifyWd);
		close(inotifyFd);
	}

	if (0 < watcherThreadId)
	{
		threadCancel(watcherThreadId);
		threadJoin(watcherThreadId);
		watcherThreadId = 0;
	}
}

void UserAppVersionHandler::onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen,
	const void* pData)
{
	_log("[UserAppVersionHandler] I ignore everything passed to onReceiveMessage()");
}

void UserAppVersionHandler::reloadNewestApkInfo()
{
	int largestVersionCode = -1;
	std::string newestApkName;

	DIR* dirp = opendir(apkDir.c_str());
	struct dirent * dp;

	while ((dp = readdir(dirp)) != NULL) 
	{
	    char *name = dp->d_name;
	    if(HiddenUtility::strEndsWith(name, ".apk"))
	    {
	    	bool ok = apkQuierer->extractVersionFromApk(apkDir + "/" + name);
	    	if (ok)
	    	{
	    		int versionCode = apkQuierer->getVersionCode();
	    		if (versionCode > largestVersionCode)
				{
					largestVersionCode = versionCode;
					newestApkName = name;
				}
	    	}
	    }
	}

	closedir(dirp);

	if (largestVersionCode > 0)
	{
		apkQuierer->extractVersionFromApk(apkDir + "/" + newestApkName);
		versionCode = apkQuierer->getVersionCode();
		versionName = apkQuierer->getVersionName();
		downloadLink = downloadLinkBasePath + "/" + newestApkName;
		lastUpdated = HiddenUtility::unixTimeMilli();

		_log("[UserAppVersionHandler] reloadNewestApkInfo() ok, packageName = %s, versionCode = %d, versionName = %s, downloadLink = %s",
		packageName.c_str(), versionCode, versionName.c_str(), downloadLink.c_str());
	}
	else
	{
		_log("[UserAppVersionHandler] reloadNewestApkInfo() failed");
	}
}

void UserAppVersionHandler::reloadConfig()
{
	std::string content, line;
	std::ifstream file(configDir + "/" + configName);
	
	while (file && std::getline(file, line))
	{
		content += line;
	}
	
	file.close();

	JSONObject configJson(content);
	if (!configJson.isValid())
	{
		_log("[UserAppVersionHandler] reloadConfig() bad content");
		return;
	}

	std::string packageName = configJson.getString("name", "");
	int versionCode = configJson.getInt("versionCode", -1);
	std::string versionName = configJson.getString("versionName", "");
	std::string downloadLink = configJson.getString("downloadLink", "");

	if (packageName.empty() || versionCode < 0 || versionName.empty() || downloadLink.empty())
	{
		_log("[UserAppVersionHandler] reloadConfig() json parsing failed");
		return;
	}

	this->packageName = packageName;
	this->versionCode = versionCode;
	this->versionName = versionName;
	this->downloadLink = downloadLink;
	this->lastUpdated = HiddenUtility::unixTimeMilli();

	_log("[UserAppVersionHandler] reloadConfig() ok, packageName = %s, versionCode = %d, versionName = %s, downloadLink = %s",
		packageName.c_str(), versionCode, versionName.c_str(), downloadLink.c_str());
}

std::string UserAppVersionHandler::getPackageName()
{
	if (useConfigWatcherMethod)
	{
		// 這時的 packageName 是從 config 檔案讀取的變數
		return lastUpdated > 0 ? this->packageName : "";
	}
	else if(useApkScanningMethod)
	{
		// 這時的 packageName 是類別初始化時就確定的常數
		return this->packageName;
	}

	return "??????";
}

int UserAppVersionHandler::getVersionCode()
{
	return lastUpdated > 0 ? this->versionCode : 0;
}

std::string UserAppVersionHandler::getVersionName()
{
	return lastUpdated > 0 ? this->versionName : "0.0.0";
}

std::string UserAppVersionHandler::getDownloadLink()
{
	return lastUpdated > 0 ? this->downloadLink : "";
}
