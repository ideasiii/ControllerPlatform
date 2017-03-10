#pragma once

#include <string>
#include <map>

#include "CSocketClient.h"
#include <string>
#include "CThreadHandler.h"
using namespace std;

class CCmpHandler;
class CSocketClient;

class CClientControllerMongoDB: public CSocketClient
{
public:
	void onReceive(const int nSocketFD, const void *pData);
public:
	static CClientControllerMongoDB * getInstance();
	virtual ~CClientControllerMongoDB();
	int startClient(string strIP, const int nPort, const int nMsqId);
	void stopClient();
	void runEnquireLinkRequest();
	int sendCommand(void * param);
	int sendCommand(int commandID, int seqNum);

private:
	CClientControllerMongoDB();
	CCmpHandler *cmpParser;

	typedef int (CClientControllerMongoDB::*MemFn)(int, int, int, const void *);
	map<int, MemFn> mapFunc;

	int cmpAccessLogRequest(string strType, string strLog);
	int cmpAccessLogResponse(int nSocket, int nCommand, int nSequence, const void *pData);

	int cmpEnquireLinkRequest(const int nSocketFD);

	int cmpEnquireLinkResponse(int nSocket, int nCommand, int nSequence, const void *pData);

	CThreadHandler *tdEnquireLink;
};
