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
	typedef struct _SOCKET_CLIENT
	{
		unsigned long int ulReceiveThreadID;
		long int ulAliveTime;
	} SOCKET_CLIENT;

public:
	int start(const char* cszAddr, short nPort);
	void stop();
	void closeClient(int nClientFD);
	void setIdleTimeout(int nSeconds);

	/**
	 * Below function is called by thread
	 */
public:
	void runSocketAccept();
	void runMessageReceive();
	void runTcpReceive();

	/**
	 * Be called by message queue run
	 */
protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

	/**
	 * Overload function
	 */
protected:
	virtual void onTimer(int nId) = 0;
	virtual void onReceive(unsigned long int nId, int nDataLen, const void* pData) = 0;

private:
	void checkIdle();

private:
	int mnMsqKey; // Message queue key and filter ID.
	unsigned long munRunThreadId; // Message queue run thread ID.
//	std::map<unsigned long int, unsigned long int> mapClientThread; // Socket client receive map <client socket FD , thread ID>
//	std::map<unsigned long int, long int> mapClientAlive; // Socket client alive map <client socket FD , idle second>
	unsigned long int getClientSocketFD(unsigned long int unThreadId);
	std::map<unsigned long int, SOCKET_CLIENT> mapClient;
};
