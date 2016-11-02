/*
 * CServerDevice.h
 *
 *  Created on: 2016年8月9日
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
	void onReceive(const int nSocketFD, const void *pData, CBFun cbfun);
public:
	static CServerDevice * getInstance();
	virtual ~CServerDevice();
	int startServer(const int nPort, const int nMsqId);
	void stopServer();
	void setCallback(const int nId, CBFun cbfun);

private:
	CServerDevice();
	CCmpHandler *cmpParser;
	typedef int (CServerDevice::*MemFn)(int, int, int, const void *);
	map<int, MemFn> mapFunc;
	int cmpAmxControl(int nSocket, int nCommand, int nSequence, const void *pData);
	map<int, CBFun> mapCallback;
};
