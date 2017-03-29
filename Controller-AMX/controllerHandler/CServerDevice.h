/*
 * CServerDevice.h
 *
 *  Created on: 2016撟�8���9�
 *      Author: root
 */

#pragma once

#include <map>

#include "CCmpServer.h"
#include "iCommand.h"
#include "ICallback.h"

class CServerDevice: public CCmpServer
{

public:
	void onReceive(const int nSocketFD, const void *pData);
public:
	static CServerDevice * getInstance();
	virtual ~CServerDevice();
	void setCallback(const int nId, CBFun cbfun);
	void broadcastAMXStatus(const char *szStatus);
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);
	void setAmxBusyTimeout(int nSec);

protected:
	void onTimer(int nId);

private:
	CServerDevice();
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
