#include "UserConfigFileAppVersionHandler.h"

#include <errno.h>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include "HiddenUtility.hpp"
#include "LogHandler.h"
#include "JSONObject.h"

#define INOTIFY_WATCH_EVENT IN_CLOSE_WRITE

UserConfigFileAppVersionHandler::UserConfigFileAppVersionHandler(
	std::string configDir, std::string configName) :
		UserAppVersionHandler(configDir, INOTIFY_WATCH_EVENT), configName(configName)
{
}

bool UserConfigFileAppVersionHandler::onInotifyEvent(struct inotify_event *event)
{
	if (event->mask & INOTIFY_WATCH_EVENT
		&& strcmp(configName.c_str(), (char*)event->name) == 0
		&& HiddenUtility::unixTimeMilli() - lastUpdated > 1000)
	{
		_log("[UserConfigFileAppVersionHandler] Config changed, reloading");
		reload();
		return false;
	}

	return true;
}

void UserConfigFileAppVersionHandler::reload()
{
	std::string content, line;
	std::ifstream file(watchDir + "/" + configName);
	
	while (file && std::getline(file, line))
	{
		content += line;
	}
	
	file.close();

	JSONObject configJson(content);
	if (!configJson.isValid())
	{
		_log("[UserConfigFileAppVersionHandler] reload() bad content");
		return;
	}

	std::string packageName = configJson.getString("name", "");
	int versionCode = configJson.getInt("versionCode", -1);
	std::string versionName = configJson.getString("versionName", "");
	std::string downloadLink = configJson.getString("downloadLink", "");

	if (packageName.empty() || versionCode < 0 || versionName.empty() || downloadLink.empty())
	{
		_log("[UserConfigFileAppVersionHandler] reload() json parsing failed");
		return;
	}

	this->packageName = packageName;
	this->versionCode = versionCode;
	this->versionName = versionName;
	this->downloadLink = downloadLink;
	this->lastUpdated = HiddenUtility::unixTimeMilli();

	_log("[UserConfigFileAppVersionHandler] reload() ok, packageName = %s, versionCode = %d, versionName = %s, downloadLink = %s",
		packageName.c_str(), versionCode, versionName.c_str(), downloadLink.c_str());
}

std::string UserConfigFileAppVersionHandler::getPackageName()
{
	// 這時的 packageName 是從 config 檔案讀取的變數
	return lastUpdated > 0 ? packageName : "";
}

int UserConfigFileAppVersionHandler::getVersionCode()
{
	return lastUpdated > 0 ? versionCode : 0;
}

std::string UserConfigFileAppVersionHandler::getVersionName()
{
	return lastUpdated > 0 ? versionName : "0.0.0";
}

std::string UserConfigFileAppVersionHandler::getDownloadLink()
{
	return lastUpdated > 0 ? downloadLink : "";
}
