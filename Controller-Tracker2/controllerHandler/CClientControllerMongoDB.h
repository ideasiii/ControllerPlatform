#pragma once

#include <string>
#include <map>

#include "CCmpClient.h"
#include <string>
#include "CThreadHandler.h"
using namespace std;

class CCmpHandler;
class CSocketClient;
class DynamicField;

class CClientControllerMongoDB: public CCmpClient
{
public:
	CClientControllerMongoDB(CObject * object);
	virtual ~CClientControllerMongoDB();
	int sendCommand(const void * param);
	int sendCommand(int commandID, int seqNum);

private:

	int cmpAccessLogRequest(string strType, string strLog);
	int cmpEnquireLinkRequest(const int nSocketFD);

	//int onAccesslogResponse(int nSocket, int nCommand, int nSequence, const void *szBody);
	//int onEnquireLinkResponse(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody);

	CCmpHandler *cmpParser;
	DynamicField * dynamicField;

	CObject *mpController;



};
