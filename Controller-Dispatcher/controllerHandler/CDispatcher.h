/*
 * CDispatcher.h
 *
 *  Created on: 2017年3月7日
 *      Author: root
 */

#pragma once

#include <map>
#include "CSocketServer.h"

class CCmpHandler;

class CDispatcher: public CSocketServer
{
public:
	static CDispatcher* getInstance();
	virtual ~CDispatcher();
	int startServer(const char *szIP, const int nPort, const int nMsqId);
	void stopServer();
	void onReceiveMessage(unsigned long int nSocketFD, int nDataLen, const void* pData);
	void setClient(unsigned long int nId, bool bAdd);
	void checkClient();

private:
	explicit CDispatcher();
	std::map<unsigned long int, long> listClient;
	CCmpHandler *cmpParser;
	typedef int (CDispatcher::*MemFn)(int, int, int, const void *);
	std::map<int, MemFn> mapFunc;
	int cmpInitial(int nSocket, int nCommand, int nSequence, const void *pData);
	long getTick();
};
