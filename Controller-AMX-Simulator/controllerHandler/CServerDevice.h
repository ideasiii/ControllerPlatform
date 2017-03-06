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

class CCmpHandler;

class CServerDevice: public CSocketServer
{
public:
	void onReceive(const int nSocketFD, const void *pData);
public:
	static CServerDevice * getInstance();
	virtual ~CServerDevice();
	int startServer(std::string strIP, const int nPort, const int nMsqId);
	void stopServer();
	void setCallback(const int nId, CBFun cbfun);
	void broadcastAMXStatus(std::string strStatus);
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);
	void onTimer(int nId);
	void setAmxBusyTimeout(int nSec);

private:
	CServerDevice();
	CCmpHandler *cmpParser;
	typedef int (CServerDevice::*MemFn)(int, int, int, const void *);
	std::map<int, MemFn> mapFunc;
	int cmpAmxControl(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAmxStatus(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpBind(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpUnbind(int nSocket, int nCommand, int nSequence, const void *pData);
	std::map<int, CBFun> mapCallback;
	std::map<int, int> mapClient;
	volatile int mnBusy;
	int mAmxBusyTimeout;

};
