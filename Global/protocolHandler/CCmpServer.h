/*
 * CCmpServer.h
 *
 *  Created on: 2017年3月16日
 *      Author: Jugo
 */

#pragma once

#include <map>
#include "CATcpServer.h"

class CCmpServer: public CATcpServer
{
public:
	CCmpServer();
	virtual ~CCmpServer();
	int request(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData);
	int response(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData);
	void idleTimeout(bool bRun, int nIdleTime);

protected:
	void onTimer(int nId);
	void onReceive(unsigned long int nSocketFD, int nDataLen, const void* pData);
	int onTcpReceive(unsigned long int nSocketFD);

	/**
	 * Controller Message Protocol (CMP) Request Callback.
	 */
protected:
	virtual int onInitial(int nSocket, int nCommand, int nSequence, const void *szData)
	{
		return 0;
	}
	;
	virtual int onSignin(int nSocket, int nCommand, int nSequence, const void *szData)
	{
		return 0;
	}
	;
	virtual int onAccesslog(int nSocket, int nCommand, int nSequence, const void *szData)
	{
		return 0;
	}
	;

private:
	typedef int (CCmpServer::*MemFn)(int, int, int, const void *);
	std::map<int, MemFn> mapFunc;
	int sendPacket(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData);

};
