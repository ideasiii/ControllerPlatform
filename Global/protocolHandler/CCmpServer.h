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

protected:
	void onTimer(int nId);
	void onReceive(unsigned long int nId, int nDataLen, const void* pData);

protected:
	virtual int onSignin(int nSocket, int nCommand, int nSequence, const void *pData) = 0;
	virtual int onAccesslog(int nSocket, int nCommand, int nSequence, const void *pData) = 0;

private:
	typedef int (CCmpServer::*MemFn)(int, int, int, const void *);
	std::map<int, MemFn> mapFunc;

};
