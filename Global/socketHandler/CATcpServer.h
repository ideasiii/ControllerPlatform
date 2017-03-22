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
	void eraseClient(unsigned long int ulSocketFD);
	void updateClientAlive(unsigned long int ulSocketFD);

private:
	int mnMsqKey; // Message queue key and filter ID.
	unsigned long munRunThreadId; // Message queue run thread ID.
	unsigned long int getClientSocketFD(unsigned long int unThreadId);
	unsigned long int getClientThreadID(unsigned long int unSocketFD);
	std::map<unsigned long int, SOCKET_CLIENT> mapClient;
};
