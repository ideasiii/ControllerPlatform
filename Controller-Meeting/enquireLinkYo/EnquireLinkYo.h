#pragma once

#include <atomic>
#include <string>
#include "CObject.h"

class CCmpClient;

// 我只是想幫忙：協助處理 enquire link 發送以及狀況處理的類別
class EnquireLinkYo : public CObject
{
public:
	EnquireLinkYo(std::string taskName, CCmpClient *client);
	~EnquireLinkYo();

	// 設定發送 enquire link 的間隔
	void setRequestInterval(int value);

	// 設定最大的 enquire link 容許消失值
	// 當沒有收到的 enquire link response 達到 value，就當作已經斷線惹
	void setMaxBalance(int value);

	// 開始對 sockFd 發送 enquire link request
	void start();
	void stop();

	// 將沒收到的 enquire link response 的計數器減 1
	void decreaseBalance();
	
private:
	std::string strTaskName;
	CCmpClient *client;
	pthread_t loopThreadId;
	std::atomic_bool isRunning;

	int requestInterval;
	int maxBalance;
	std::atomic_int balance;
	
	void run();

	friend void *threadStartRoutine_EnquireLinkYo_run(void *argv);
};
