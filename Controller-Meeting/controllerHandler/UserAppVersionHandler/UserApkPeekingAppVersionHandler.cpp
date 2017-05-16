#include "UserApkPeekingAppVersionHandler.h"

#include <dirent.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "../HiddenUtility.hpp"
#include "AndroidPackageInfoQuierer.hpp"
#include "LogHandler.h"

#define INOTIFY_WATCH_EVENT IN_CLOSE_WRITE

UserApkPeekingAppVersionHandler::UserApkPeekingAppVersionHandler
	(AndroidPackageInfoQuierer *q, std::string pkgName, std::string apkDir, std::string dlLinkBase) :
		UserAppVersionHandler(apkDir, INOTIFY_WATCH_EVENT), apkQuierer(q), 
		downloadLinkBasePath(dlLinkBase)
{
	packageName = pkgName;
}

UserApkPeekingAppVersionHandler::~UserApkPeekingAppVersionHandler()
{
}

bool UserApkPeekingAppVersionHandler::onInotifyEvent(struct inotify_event *event)
{
	if (event->mask & INOTIFY_WATCH_EVENT
		&& HiddenUtility::strEndsWith((char*)event->name, ".apk")
		&& HiddenUtility::unixTimeMilli() - lastUpdated > 1000)
	{
		_log("[UserApkPeekingAppVersionHandler] APK created, reloading");
		reload();
		return false;
	}

	return true;
}

void UserApkPeekingAppVersionHandler::reload()
{
	_log("[UserApkPeekingAppVersionHandler] reload() get version of apks in `%s`", watchDir.c_str());

	int largestVersionCode = -1;
	std::string newestApkName;

	DIR* dirp = opendir(watchDir.c_str());
	if (dirp == NULL)
	{
		_log("[UserApkPeekingAppVersionHandler] reload() opendir() failed: %s", strerror(errno));
		return;
	}

	struct dirent * dp;

	while ((dp = readdir(dirp)) != NULL) 
	{
	    char *name = dp->d_name;
	    if(HiddenUtility::strEndsWith(name, ".apk"))
	    {
	    	bool ok = apkQuierer->extractVersionFromApk(watchDir + "/" + name);
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
		apkQuierer->extractVersionFromApk(watchDir + "/" + newestApkName);
		
		versionCode = apkQuierer->getVersionCode();
		versionName = apkQuierer->getVersionName();
		downloadLink = downloadLinkBasePath + "/" + newestApkName;
		lastUpdated = HiddenUtility::unixTimeMilli();

		_log("[UserApkPeekingAppVersionHandler] reload() ok, packageName = %s, versionCode = %d, versionName = %s, downloadLink = %s",
		packageName.c_str(), versionCode, versionName.c_str(), downloadLink.c_str());
	}
	else
	{
		_log("[UserApkPeekingAppVersionHandler] reload() failed");
	}
}

std::string UserApkPeekingAppVersionHandler::getPackageName()
{
	// 這時的 packageName 是類別初始化時就確定的常數
	return packageName;
}

int UserApkPeekingAppVersionHandler::getVersionCode()
{
	return lastUpdated > 0 ? versionCode : 0;
}

std::string UserApkPeekingAppVersionHandler::getVersionName()
{
	return lastUpdated > 0 ? versionName : "0.0.0";
}

std::string UserApkPeekingAppVersionHandler::getDownloadLink()
{
	return lastUpdated > 0 ? downloadLink : "";
}
