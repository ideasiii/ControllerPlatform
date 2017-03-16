/*
 * CATcpServer.h
 *
 *  Created on: 2017年3月15日
 *      Author: Jugo
 *
 *  This is a abstract class for socket server
 */

#pragma once

#include <map>
#include "CSocket.h"
#include "CObject.h"

class CATcpServer: public CSocket, public CObject
{
public:
	int start(const char* cszAddr, short nPort);
	void runSocketAccept();
	void runMessageReceive();
	void runTcpReceive();
	void stop();

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

protected:
	virtual void onTimer(int nId) = 0;
	virtual void onReceive(unsigned long int nId, int nDataLen, const void* pData) = 0;

private:
	int mnMsqKey;
	unsigned long munRunThreadId;
	std::map<unsigned long int, unsigned long int> mapClientThread; // <client socket FD , thread ID>
	unsigned long int getClientSocketFD(unsigned long int unThreadId);

};
