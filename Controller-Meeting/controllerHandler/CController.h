#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "CApplication.h"

class CClientAmxController;
class CClientMeetingAgent;
class CConfig;
class CThreadHandler;

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();

protected:
	// return message queue key here
	int onCreated(void* nMsqKey) override;

	// allocate resources here
	int onInitial(void* szConfPath) override;

	// release resources here
	int onFinish(void* nMsqKey) override;

	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData) override;
	void onHandleMessage(Message &message) override;

	std::string taskName() override;

private:
	int mnMsqKey;
	std::unique_ptr<CClientMeetingAgent> agentClient;
	std::unique_ptr<CClientAmxController> amxControllerClient;

	std::atomic<pthread_t> agentConnectingThreadId;
	std::atomic<pthread_t> amxReconnectThreadId;

	//int initAgentClient();
	int initAgentClient(std::unique_ptr<CConfig>& config);

	// 啟動與伺服器連線的 thread
	void startConnectToAgentThread();
	void startConnectToAmxThread();

	// 用來與伺服器連線的 thread
	friend void *threadStartRoutine_CController_connectToAgent(void *argv);
	friend void *threadStartRoutine_CController_connectToAmx(void *argv);
};
