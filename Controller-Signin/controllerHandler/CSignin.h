/*
 * CSignin.h
 *
 *  Created on: 2017年3月14日
 *      Author: Jugo
 */

#pragma once
#include <map>
#include "CSocketServer.h"

class CCmpHandler;

class CSignin: public CSocketServer
{
public:
	static CSignin* getInstance();
	virtual ~CSignin();
	int startServer(const char *szIP, const int nPort, const int nMsqId);
	void stopServer();
	void onReceiveMessage(unsigned long int nSocketFD, int nDataLen, const void* pData);
	void setClient(unsigned long int nId, bool bAdd);
	void checkClient();

protected:
	void onTimer(int nId);

private:
	explicit CSignin();
	std::map<unsigned long int, long> listClient;
	CCmpHandler *cmpParser;
	typedef int (CSignin::*MemFn)(int, int, int, const void *);
	std::map<int, MemFn> mapFunc;
	int cmpSignin(int nSocket, int nCommand, int nSequence, const void *pData);
};
