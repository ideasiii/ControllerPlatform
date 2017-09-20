#include "AppVersionHandlerFactory.h"

#include <string>
#include "AndroidPackageInfoQuierer.hpp"
#include "ApkPeekingAppVersionHandler.h"
#include "AppVersionHandler.h"
#include "CConfig.h"
#include "ConfigFileAppVersionHandler.h"
#include "LogHandler.h"

#define LOG_TAG "[AppVersionHandlerFactory]"
#define CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG "APP DOWNLOAD INFO CONFIG WATCHER"

AppVersionHandler* AppVersionHandlerFactory::createFromConfig(std::unique_ptr<CConfig> &config)
{
	std::string strAaptPath = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "aapt_path");
	std::string strApkDir = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "apk_dir");
	std::string strPkgName = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "package_name");
	std::string strDownloadLinkBase = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "download_link_base");

	if (!strAaptPath.empty() && !strApkDir.empty()
		&& !strPkgName.empty() && !strDownloadLinkBase.empty())
	{
		_log(LOG_TAG" init ApkPeekingAppVersionHandler");
		auto apkQuierer = new AndroidPackageInfoQuierer(strAaptPath, strPkgName);
		return new ApkPeekingAppVersionHandler(apkQuierer, strPkgName, strApkDir, strDownloadLinkBase);
	}

	std::string strAppDownloadLinkConfigDir = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "config_dir");
	std::string strAppDownloadLinkConfigName = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "config_name");

	if (!strAppDownloadLinkConfigDir.empty()
		&& !strAppDownloadLinkConfigName.empty())
	{
		_log(LOG_TAG" init ConfigFileAppVersionHandler");
		return new ConfigFileAppVersionHandler(
			strAppDownloadLinkConfigDir, strAppDownloadLinkConfigName);
	}

	_log(LOG_TAG" createFromConfig() cannot spawn AppVersionHandler");
	return nullptr;
}

AppVersionHandlerFactory::AppVersionHandlerFactory()
{
}

AppVersionHandlerFactory::~AppVersionHandlerFactory()
{
}
