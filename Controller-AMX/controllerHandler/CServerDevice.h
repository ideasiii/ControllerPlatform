/*
 * CServerDevice.h
 *
 *  Created on: 2016年8月9日
 *      Author: root
 */

#pragma once

#include <string>

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

private:
	CServerDevice();
	CCmpHandler *cmpParser;
};
