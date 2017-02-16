
#pragma once

#include <string>
#include <map>

#include "CSocketServer.h"
#include "ICallback.h"

using namespace std;

class CCmpHandler;

class CServerDevice: public CSocketServer
{
public:
	void onReceive(const int nSocketFD, const void *pData);
public:
	static CServerDevice * getInstance();
	virtual ~CServerDevice();
	int startServer(string strIP, const int nPort, const int nMsqId);
	void stopServer();


private:
	CServerDevice();
	CCmpHandler *cmpParser;
	typedef int (CServerDevice::*MemFn)(int, int, int, const void *);
	map<int, MemFn> mapFunc;
	int cmpFCMIdRegister(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpFBToken(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpQRCodeToken(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAPPVersion(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpGetMeetingData(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpWirelessPowerCharge(int nSocket, int nCommand, int nSequence, const void *pData);


};
