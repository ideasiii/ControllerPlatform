#pragma once

#include <string>
#include <vector>
#include <map>
#include "CApplication.h"
#include "common.h"
#include "packet.h"

using namespace std;

class CThreadHandler;
class CClientMeetingAgent;

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();

protected:
	// return message queue key here
	int onCreated(void* nMsqKey);

	// allocate resources here
	int onInitial(void* szConfPath);

	// release resources here
	int onFinish(void* nMsqKey);

	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
	virtual void onHandleMessage(Message &message);
private:
	int mnMsqKey; 
	CClientMeetingAgent *mCClientMeetingAgent;

	CThreadHandler *tdEnquireLink;
	CThreadHandler *tdExportLog;
	std::vector<int> vEnquireLink;

	int startClientMeetingAgent(string strIP, const int nPort, const int nMsqKey);
};
