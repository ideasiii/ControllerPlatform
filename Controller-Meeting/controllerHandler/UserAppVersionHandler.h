#include <memory>
#include "CObject.h"

class AndroidPackageInfoQuierer;

// 處理使用者裝置 app 版本更新檢查要求的類別 
class UserAppVersionHandler : CObject
{
public:
	explicit UserAppVersionHandler(std::string dir, std::string name);
	explicit UserAppVersionHandler(AndroidPackageInfoQuierer *q, std::string pkgName, std::string dir, std::string dlLinkBase);
	virtual ~UserAppVersionHandler();

	// 根據初始化時傳入的內容啟動對應的 watcher thread
	void start();

	// stop runConfigChangeWatcher() if run in a thread
	void stop();

	// 在目前的執行緒執行 config 檔監控
	// 沒事不要用，應該呼叫 start() 以便根據建構時的類型自動選擇 watcher 方法
	void runConfigChangeWatcher();

	// 在目前的執行緒執行 apk 檔監控
	// 沒事不要用，應該呼叫 start() 以便根據建構時的類型自動選擇 watcher 方法
	void runNewApkWatcher();
	
	std::string getPackageName();
	int getVersionCode();
	std::string getVersionName();
	std::string getDownloadLink();

protected:
	virtual void onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen,
		const void* pData);
	
private:
	// path of config path, config file contains a JSON object 
	// stores download link and version info about user app
	const bool useConfigWatcherMethod;
	const std::string configDir;
	const std::string configName;

	const bool useApkScanningMethod;
	const std::string apkDir;
	std::unique_ptr<AndroidPackageInfoQuierer> apkQuierer;
	std::string apkName;
	const std::string downloadLinkBasePath;
	
	// Android apk 的 package name
	std::string packageName;
	
	// Android apk 的 version code (a.k.a. build number)
	int versionCode;
	
	// Android apk 的版本名稱
	std::string versionName;
	
	// Android apk 的 下載網址
	std::string downloadLink;

	// apk info 最後更新時間
	int64_t lastUpdated;

	pthread_t watcherThreadId;
	int stopSignalPipeFd[2];

	// run runConfigChangeWatcher() in a thread
	void startConfigChangeWatcher();

	// run runNewApkWatcher() in a thread
	void startNewApkWatcher();

	// 重新讀取 config 檔並更新此類別的成員數值
	void reloadConfig();

	// 尋找最新版本的 apk，並取得其版本內容，以此更新此類別的成員數值
	void reloadNewestApkInfo();
};
