#pragma once

#include <atomic>
#include <string>
#include "CObject.h"

class CCmpClient;

// 我只是想幫忙：協助處理 enquire link 發送以及對 Controller 發送斷線事件的類別
class EnquireLinkYo : public CObject
{
public:
	EnquireLinkYo(std::string whoUsedMe, CCmpClient *client, int nCommandOnDisconnect, CObject *controller);
	~EnquireLinkYo();

	// 設定發送 enquire link 的間隔
	void setRequestInterval(int value);

	// 設定最大的 enquire link 容許消失值
	// 當沒收到的 response 數量達到 value，就當作已經從伺服器斷線惹
	void setMaxBalance(int value);

	// 開始發送 enquire link request
	void start();
	void stop();

	// 將沒收到的 enquire link response 計數器歸零
	void zeroBalance();

protected:
	void onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen, const void* pData) override;
	void onHandleMessage(Message &message) override;

	// 因為這個 class 是使用 composition 的方式被 CCmpClient 使用的，無法 override taskName
	// 所以只好再用一個 yoIdentifier，在建構的時候指定 task name
	std::string taskName() override;

private:
	// string returned in taskName()
	// 最好不要超過 15 字元 (PR_SET_NAME 限制)
	const std::string yoIdentifier;
	
	// do not delete this or this class will die with it
	CCmpClient * const client;
	// do not delete this or the whole world will collapse
	CObject * const mpController;

	// Command to be filled by mpController when informing broken pipe
	const int commandOnDisconnect;

	pthread_t loopThreadId;
	std::atomic_bool isRunning;

	int requestInterval;
	int maxBalance;
	std::atomic_int balance;
	
	// main loops used to send and monitor enquire link requests
	void run();

	// 通知 mpController enquire link 失敗
	void informEnquireLinkFailure();

	friend void *threadStartRoutine_EnquireLinkYo_run(void *argv);
};
