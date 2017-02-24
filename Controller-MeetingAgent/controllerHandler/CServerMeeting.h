/*
 * CServerAMX.h
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#pragma once

#include "CSocketServer.h"
#include <string>
#include <map>
#include "ICallback.h"
#include "CMPData.h"

#include "CThreadHandler.h"
class CCmpHandler;

using namespace std;

class CServerMeeting: public CSocketServer
{
public:
	static CServerMeeting * getInstance();
	virtual ~CServerMeeting();
	int startServer(string strIP, const int nPort, const int nMsqId);
	void stopServer();
	void onReceive(const int nSocketFD, const void *pData);
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);
	int sendCommand(int commandID, int seqNum, string bodyData);
	void setCallback(const int nId, CBFun cbfun);
	void runEnquireLinkRequest();
private:

	int cmpBind(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpUnbind(int nSocket, int nCommand, int nSequence, const void *pData);

	CServerMeeting();

	typedef int (CServerMeeting::*MemFn)(int, int, int, const void *);
	map<int, MemFn> mapFunc;

	vector<int> mapClient;
	CCmpHandler *cmpParser;
	map<int, CBFun> mapCallback;

	//for Controller-Meeting Data
	int cmpQRCodeToken(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAPPVersion(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpGetMeetingData(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *pData);
	CMPData parseCMPData(int nSocket, int nCommand, int nSequence, const void *pData, bool isBodyExist);

	int cmpEnquireLinkRequest(const int nSocketFD);
	int getBindSocket(vector<int> &listValue);

	int cmpEnquireLinkResponse(int nSocket, int nCommand, int nSequence, const void *pData);


	CThreadHandler *tdEnquireLink;

};
