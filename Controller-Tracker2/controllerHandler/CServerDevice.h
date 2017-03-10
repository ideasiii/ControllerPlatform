
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


	void setCallback(const int nId, CBFun cbfun);

	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);

	void sendCommand(int socketFD, int commandID, int seqNum, string bodyData);
private:
	CServerDevice();
	CCmpHandler *cmpParser;
	typedef int (CServerDevice::*MemFn)(int, int, int, const void *);
	map<int, MemFn> mapFunc;


	int cmpAccessLogRequest(int nSocket, int nCommand, int nSequence, const void *pData);

	map<int, CBFun> mapCallback;
	map<int, int> mapClient;

	int paseBody(const void *pData, CDataHandler<string> &rData);
	int getLength(const void *pData);



};
