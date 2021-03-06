/*
 *  CServerCenter.h
 *
 *  Created on: 2016-12-05
 *  Author: Jugo
 */

#pragma once

#include <map>
#include "CSocketServer.h"

class CCmpHandler;

class CServerCenter: public CSocketServer
{
public:
	void onReceive(const int nSocketFD, const void *pData);
public:
	static CServerCenter * getInstance();
	virtual ~CServerCenter();
	int startServer(const char *szIP, const int nPort, const int nMsqId);
	void stopServer();
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);

private:
	explicit CServerCenter();
	CCmpHandler *cmpParser;
	typedef int (CServerCenter::*MemFn)(int, int, int, const void *);
	std::map<int, MemFn> mapFunc;
	int cmpBind(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpUnbind(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpInitial(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpSignup(int nSocket, int nCommand, int nSequence, const void *pData);
	std::map<int, int> mapClient;

};
