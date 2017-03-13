
#pragma once

#include <string>
#include <map>

#include "CSocketServer.h"
#include "CMPData.h"
#include "iCommand.h"

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


	void setCallback(const int nId, CBFun cbfun);
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);
	void sendCommand(int socketFD, int commandID, int seqNum, string bodyData);
private:
	CServerDevice();
	CCmpHandler *cmpParser;
	typedef int (CServerDevice::*MemFn)(int, int, int, const void *);
	map<int, MemFn> mapFunc;

	CMPData parseCMPData (int nSocket, int nCommand, int nSequence, const void *pData,bool isBodyExist);

	//for other Controller Data
	int cmpFCMIdRegister(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpFBToken(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpWirelessPowerCharge(int nSocket, int nCommand, int nSequence, const void *pData);

	//for Controller-Meeting Data
	int cmpQRCodeToken(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAPPVersion(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpGetMeetingData(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *pData);

	map<int, CBFun> mapCallback;
	map<int, int> mapClient;



};
