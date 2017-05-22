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
	// 當沒有收到的 enquire link response 達到 value，就當作已經斷線惹
	void setMaxBalance(int value);

	// 開始對 sockFd 發送 enquire link request
	void start();
	void stop();

	// 將沒收到的 enquire link response 的計數器歸零
	void zeroBalance();

protected:
	void onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen, const void* pData) override;
	void onHandleMessage(Message &message) override;

	// 因為這個 class 是使用 composition 的方式被 CCmpClient 使用的，無法 override taskName
	// 所以只好再用一個 strTaskName，在建構的時候指定 task name
	std::string taskName() override;

private:
	const std::string whoUsedMe;
	CCmpClient * client;
	CObject * mpController;

	// Value to be filled in Message.what when informing outside
	const int commandOnDisconnect;

	pthread_t loopThreadId;
	std::atomic_bool isRunning;

	int requestInterval;
	int maxBalance;
	std::atomic_int balance;
	
	void run();

	// 通知外面 enquire link 失敗
	void informEnquireLinkFailure();

	friend void *threadStartRoutine_EnquireLinkYo_run(void *argv);
};
