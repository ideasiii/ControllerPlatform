#pragma once

#include <string>
#include <vector>
#include <list>
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
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	int mnMsqKey; 
	CClientMeetingAgent *mCClientMeetingAgent;

	CThreadHandler *tdEnquireLink;
	CThreadHandler *tdExportLog;
	std::vector<int> vEnquireLink;

	int startClientMeetingAgent(string strIP, const int nPort, const int nMsqKey);
};
