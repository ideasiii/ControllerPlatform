#pragma once

#include <string>
#include <map>

#include "CSocketClient.h"
#include <string>
#include "CThreadHandler.h"
using namespace std;

class CCmpHandler;
class CSocketClient;
class DynamicField;

class CClientControllerMongoDB: public CSocketClient
{
public:
	void onReceive(const int nSocketFD, const void *pData);
public:
	static CClientControllerMongoDB * getInstance();
	virtual ~CClientControllerMongoDB();
	int startClient(string strIP, const int nPort, const int nMsqId);
	void stopClient();
	int sendCommand(void * param);
	int sendCommand(int commandID, int seqNum);

private:
	CClientControllerMongoDB();
	int cmpAccessLogRequest(string strType, string strLog);
	int cmpAccessLogResponse(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpEnquireLinkRequest(const int nSocketFD);
	int cmpEnquireLinkResponse(int nSocket, int nCommand, int nSequence, const void *pData);

	CCmpHandler *cmpParser;

	typedef int (CClientControllerMongoDB::*MemFn)(int, int, int, const void *);
	map<int, MemFn> mapFunc;
	DynamicField * dynamicField;

};
