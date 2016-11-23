/*
 * CServerDevice.h
 *
 *  Created on: 2016撟�8���9�
 *      Author: root
 */

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
	int startServer(const int nPort, const int nMsqId);
	void stopServer();
	void setCallback(const int nId, CBFun cbfun);
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);
	void broadcastAMXStatus(string strStatus);

private:
	CServerDevice();
	int cmpResponse(const int nSocket, const int nCommandId, const int nSequence, const char * szData);
	CCmpHandler *cmpParser;
	typedef int (CServerDevice::*MemFn)(int, int, int, const void *);
	map<int, MemFn> mapFunc;
	int cmpAmxControl(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAmxStatus(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpBind(int nSocket, int nCommand, int nSequence, const void *pData);
	map<int, CBFun> mapCallback;
	map<int, int> mapClient;
};
