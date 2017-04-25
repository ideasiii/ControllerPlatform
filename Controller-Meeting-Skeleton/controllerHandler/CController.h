/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>
#include "CObject.h"
#include "common.h"
#include "packet.h"

using namespace std;

class CCmpHandler;
class CThreadHandler;
class CJsonHandler;
class CClientMeetingAgent;
class CSocket;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int startClientMeetingAgent(string strIP, const int nPort, const int nMsqId);
	void stopServer();

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();

public:
	CCmpHandler *cmpParser;

private:
	CClientMeetingAgent *mCClientMeetingAgent;

	CThreadHandler *tdEnquireLink;
	CThreadHandler *tdExportLog;
	std::vector<int> vEnquireLink;
};
