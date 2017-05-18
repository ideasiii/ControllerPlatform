#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "CApplication.h"

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
	std::unique_ptr<CClientMeetingAgent> clientAgent;

	std::unique_ptr<CThreadHandler> tdEnquireLink;

	pthread_t tdEnquireLinkTid;

	friend void *threadStartRoutine_CController_enquireLink(void *args);
};
