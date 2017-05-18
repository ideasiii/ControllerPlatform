#pragma once

#include <memory>
#include "AppVersionHandler.h"

// 處理使用者裝置 app 版本更新檢查要求的類別 
class ConfigFileAppVersionHandler : public AppVersionHandler
{
public:
	explicit ConfigFileAppVersionHandler(std::string configDir, std::string configName);
	virtual ~ConfigFileAppVersionHandler();

	std::string getPackageName() override;
	int getVersionCode() override;
	std::string getVersionName() override;
	std::string getDownloadLink() override;
	std::string taskName() override;

protected:
	// 重新讀取 config 檔並更新此類別的成員數值
	void reload() override;
	bool onInotifyEvent(struct inotify_event *event) override;
	
private:
	const std::string configName;
};
