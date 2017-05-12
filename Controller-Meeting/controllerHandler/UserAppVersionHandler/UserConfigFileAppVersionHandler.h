#include <memory>
#include "UserAppVersionHandler.h"

// 處理使用者裝置 app 版本更新檢查要求的類別 
class UserConfigFileAppVersionHandler : UserAppVersionHandler
{
public:
	explicit UserConfigFileAppVersionHandler(std::string configDir, std::string configName);
	virtual ~UserConfigFileAppVersionHandler();

	virtual std::string getPackageName();
	virtual int getVersionCode();
	virtual std::string getVersionName();
	virtual std::string getDownloadLink();

protected:
	// 在目前的執行緒執行 config 檔監控
	// 沒事不要用，應該呼叫 start()
	virtual void runWatcher();

	// 重新讀取 config 檔並更新此類別的成員數值
	virtual void reload();
	virtual bool onInotifyEvent(struct inotify_event *event);
	
private:
	const std::string configName;
};
