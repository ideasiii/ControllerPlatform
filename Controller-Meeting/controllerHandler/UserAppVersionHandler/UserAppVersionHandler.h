#pragma once

#include <memory>
#include <string>
#include <sys/inotify.h>
#include "CObject.h"

// 處理使用者裝置 app 版本更新檢查要求的類別 
class UserAppVersionHandler : CObject
{
public:
	virtual ~UserAppVersionHandler();

	// 啟動 watcher thread
	void start();

	// 停止 watcher thread
	void stop();
	
	virtual std::string getPackageName() = 0;
	virtual int getVersionCode() = 0;
	virtual std::string getVersionName() = 0;
	virtual std::string getDownloadLink() = 0;

protected:
	std::string watchDir;

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

	explicit UserAppVersionHandler(std::string watchDirectory, int inotifyMask);

	// 這個類別不使用 message queue 傳遞訊息
	virtual void onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen,
		const void* pData);

	// 刷新類別內的變數
	virtual void reload() = 0;
	virtual bool onInotifyEvent(struct inotify_event *event) = 0;
private:
	const int inotifyEventMask; 
	pthread_t watcherThreadId;
	bool doLoop;
	
	// 在目前的執行緒執行 watcher
	// 沒事不要用，應該呼叫 start()
	virtual void runWatcher();

	friend void *threadStartRoutine_UserAppVersionHandler_runWatcher(void *);
};
