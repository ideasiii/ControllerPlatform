#include "ConfigFileAppVersionHandler.h"

#include <errno.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "../HiddenUtility.hpp"
#include "LogHandler.h"
#include "JSONObject.h"

#define LOG_TAG "[ConfigFileAppVersionHandler]"
#define INOTIFY_WATCH_EVENT IN_CLOSE_WRITE

ConfigFileAppVersionHandler::ConfigFileAppVersionHandler(
	std::string configDir, std::string configName) :
		AppVersionHandler(configDir, INOTIFY_WATCH_EVENT), configName(configName)
{
}

ConfigFileAppVersionHandler::~ConfigFileAppVersionHandler()
{
}

bool ConfigFileAppVersionHandler::onInotifyEvent(struct inotify_event *event)
{
	if (event->mask & INOTIFY_WATCH_EVENT
		&& strcmp(configName.c_str(), (char*)event->name) == 0
		&& HiddenUtility::unixTimeMilli() - lastUpdated > 1000)
	{
		_log(LOG_TAG" Config changed, reloading");
		reload();
		return false;
	}

	return true;
}

void ConfigFileAppVersionHandler::reload()
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
		_log(LOG_TAG" reload() bad content");
		return;
	}

	std::string packageName = configJson.getString("name", "");
	int versionCode = configJson.getInt("versionCode", -1);
	std::string versionName = configJson.getString("versionName", "");
	std::string downloadLink = configJson.getString("downloadLink", "");

	if (packageName.empty() || versionCode < 0 || versionName.empty() || downloadLink.empty())
	{
		_log(LOG_TAG" reload() json parsing failed");
		return;
	}

	this->packageName = packageName;
	this->versionCode = versionCode;
	this->versionName = versionName;
	this->downloadLink = downloadLink;
	this->lastUpdated = HiddenUtility::unixTimeMilli();

	_log(LOG_TAG" reload() ok, packageName = %s, versionCode = %d, versionName = %s, downloadLink = %s",
		packageName.c_str(), versionCode, versionName.c_str(), downloadLink.c_str());
}

std::string ConfigFileAppVersionHandler::getPackageName()
{
	// 這時的 packageName 是從 config 檔案讀取的變數
	return lastUpdated > 0 ? packageName : "";
}

int ConfigFileAppVersionHandler::getVersionCode()
{
	return lastUpdated > 0 ? versionCode : 0;
}

std::string ConfigFileAppVersionHandler::getVersionName()
{
	return lastUpdated > 0 ? versionName : "0.0.0";
}

std::string ConfigFileAppVersionHandler::getDownloadLink()
{
	return lastUpdated > 0 ? downloadLink : "";
}

std::string ConfigFileAppVersionHandler::taskName()
{
	return "ConfigWatch";
}
