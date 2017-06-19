#pragma once

#include <memory>
#include "AppVersionHandler.h"

class AndroidPackageInfoQuierer;

// 處理使用者裝置 app 版本更新檢查要求的類別
class ApkPeekingAppVersionHandler : public AppVersionHandler
{
public:
	explicit ApkPeekingAppVersionHandler(AndroidPackageInfoQuierer *q,
		std::string pkgName, std::string dir, std::string dlLinkBase);
	virtual ~ApkPeekingAppVersionHandler();

	std::string getPackageName() override;
	int getVersionCode() override;
	std::string getVersionName() override;
	std::string getDownloadLink() override;
	std::string taskName() override;

protected:
	// 尋找最新版本的 apk，並取得其版本內容，以此更新此類別的成員數值
	void reload() override;
	bool onInotifyEvent(struct inotify_event *event) override;

private:
	std::unique_ptr<AndroidPackageInfoQuierer> apkQuierer;
	std::string apkName;
	const std::string downloadLinkBasePath;
};
