/*
 * CATcpClient.h
 *
 *  Created on: May 4, 2017
 *      Author: joe
 *
 *  This is a abstract class for socket client
 */

#ifndef GLOBAL_SOCKETHANDLER_CATCPCLIENT_H_
#define GLOBAL_SOCKETHANDLER_CATCPCLIENT_H_

#pragma once

#include "CSocket.h"
#include "CObject.h"

class CATcpClient: public CSocket, public CObject
{

	typedef struct _SOCKET_SERVER
	{
		unsigned long int ulReceiveThreadID;
		long int ulAliveTime;
	} SOCKET_SERVER;

public:
	int connect(const char* cszAddr, short nPort, int nMsqKey);
	void stop();
	void closeServer();
	void runMessageReceive();
	void runTcpReceive();
	/**
	 * Be called by message queue run
	 */
protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
	void setIdleTimeout(int nSeconds);
	void runIdleTimeout(bool bRun);

	/**
	 * Overload function
	 */
protected:
	virtual void onTimer(int nId)
	{
	}
	;
	/**
	 * 自行定義封包處理層，不需實作 socket recv
	 */
	virtual void onReceive(unsigned long int nSocketFD, int nDataLen, const void* pData)
	{
	}
	;
	/**
	 *  自行定義 TCP 接收層，須實作 socket recv
	 */
	virtual int onTcpReceive(unsigned long int nSocketFD);

private:
	int IDLE_TIMEOUT ; // secons

	int mnExtMsqKey;
	void checkIdle();
	void updateClientAlive();

	int mnMsqKey; // Message queue key and filter ID.
	unsigned long munRunThreadId; // Message queue run thread ID.

	SOCKET_SERVER mSocketServer ;
};

#endif /* GLOBAL_SOCKETHANDLER_CATCPCLIENT_H_ */
