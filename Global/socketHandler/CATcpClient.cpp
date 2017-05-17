/*
 * CATcpClient.cpp
 *
 *  Created on: May 4, 2017
 *      Author: joe
 */

#include "CATcpClient.h"

#include "CMessageHandler.h"
#include <time.h>
#include "LogHandler.h"
#include "event.h"
#include "utility.h"

using namespace std;

#define IDLE_TIMER			469107

void *threadCATcpClientMessageReceive(void *argv)
{
	CATcpClient* ss = reinterpret_cast<CATcpClient*>(argv);
	ss->runMessageReceive();
	return 0;
}

void *aClientthreadTcpReceive(void *argv)
{
	CATcpClient* ss = reinterpret_cast<CATcpClient*>(argv);
	ss->runTcpReceive();
	return 0;
}

void CATcpClient::runTcpReceive()
{
	int result;
	int nSocketFD;

	nSocketFD = getSocketfd();
	if (0 >= nSocketFD)
	{
		_log("[CATcpClient] runTcpReceive Fail, Invalid Socket FD");
		sendMessage(EVENT_FILTER_SOCKET_CLIENT, EVENT_COMMAND_THREAD_EXIT, getThreadID(), 0, 0);
		threadExit();
		return;
	}

	sendMessage(EVENT_FILTER_SOCKET_CLIENT, EVENT_COMMAND_SOCKET_CONNECT, nSocketFD, 0, 0);

	while (1)
	{
		if (0 >= onTcpReceive(nSocketFD))
		{
			break;
		}
		sendMessage(EVENT_FILTER_SOCKET_CLIENT, EVENT_COMMAND_SOCKET_TCP_CONNECT_ALIVE, nSocketFD, 0, 0);
	}

	sendMessage(EVENT_FILTER_SOCKET_CLIENT, EVENT_COMMAND_SOCKET_DISCONNECT, nSocketFD, 0, 0);
	threadExit();
}

int CATcpClient::onTcpReceive(unsigned long int nSocketFD)
{
	int result;
	char pBuf[MAX_SOCKET_READ];
	void* pvBuf = pBuf;
	memset(pBuf, 0, sizeof(pBuf));
	result = socketrecv(nSocketFD, &pvBuf, 0);
	_log("[CATcpClient] onTcpReceive Result: %d Socket[%d]", result, nSocketFD);

	if (0 < result)
	{
		sendMessage(EVENT_FILTER_SOCKET_CLIENT, EVENT_COMMAND_SOCKET_CLIENT_RECEIVE, nSocketFD, result, pBuf);
	}
	return result;
}

void CATcpClient::runMessageReceive()
{
	munRunThreadId = getThreadID();
	run(EVENT_FILTER_SOCKET_CLIENT, "CATcpClient");
	threadExit();
	threadJoin(getThreadID());
	_log("[CATcpClient] runMessageReceive Stop, Thread join");
}

int CATcpClient::connect(const char* cszAddr, short nPort, int nMsqKey)
{
	int nMsgId = -1;
	int nSocketFD;
	IDLE_TIMEOUT = 10; //second
	mnExtMsqKey = FALSE;
	munRunThreadId = 0;

	if (-1 != nMsqKey)
	{
		mnMsqKey = nMsqKey;
		mnExtMsqKey = TRUE;
	}
	else
	{
		mnMsqKey = clock();
	}
	if (-1 == mnMsqKey)
	{
		mnMsqKey = 20170503;
	}

	nMsgId = initMessage(mnMsqKey, "TCP Client");

	if (-1 == nMsgId)
	{
		_log("[CATcpClient] Init Message Queue Fail");
		return -1;
	}

	if (-1 == setInetSocket(cszAddr, nPort))
	{
		_log("[CATcpClient] Set INET socket address & port fail");
		return -1;
	}

	if (-1 != createSocket(AF_INET, SOCK_STREAM))
	{

		if (-1 == connectServer())
		{
			socketClose();
			_log("[CATcpClient] Set INET socket address & port fail");
			return -1;
		}

		_log("munRunThreadId: %d", munRunThreadId);
		if (0 != munRunThreadId)
		{
			threadCancel(munRunThreadId);
			threadJoin(munRunThreadId);
			munRunThreadId = 0;
		}

		createThread(threadCATcpClientMessageReceive, this);
		createThread(aClientthreadTcpReceive, this);
		_log("[CATcpClient] Socket connect success, FD:%d", getSocketfd());

		return getSocketfd();
	}

	return -1;
}

void CATcpClient::stop()
{

	socketClose();
	/**
	 * Close Message queue run thread
	 */

//	_log("[CATcpClient] Close Message Queue START");
	if (0 < munRunThreadId)
	{
		//	_log("[CATcpClient] munRunThreadId > 0");
		threadCancel(munRunThreadId);
		threadJoin(munRunThreadId);
		munRunThreadId = 0;

		if (mnExtMsqKey == FALSE)
		{
			//	_log("[CATcpClient] closeMsg");
			CMessageHandler::closeMsg(CMessageHandler::registerMsq(mnMsqKey));
		}
	}
	else
	{
		//_log("[CATcpClient] munRunThreadId < 0");
	}
	//_log("[CATcpClient] Close Message Queue END");

	//_log("[CATcpClient] Close all Client Socket START");

	//threadCancel(getThreadID());
	//_log("[CATcpClient] Close all Client threadCancel END");
	//threadJoin(getThreadID());

	//_log("[CATcpClient] Close all Client Socket END");

}

//active close server
void CATcpClient::closeServer()
{

	sendMessage(EVENT_FILTER_SOCKET_CLIENT, EVENT_COMMAND_SOCKET_SERVER_COLSE, getSocketfd(), 0, 0);

}
void CATcpClient::checkIdle()
{
	double diff;
	diff = difftime(nowSecond(), mSocketServer.ulAliveTime);

	if (IDLE_TIMEOUT < (int) diff)
	{
		_log("[CATcpServer] Socket Client: %d idle: %d seconds", mSocketServer, (int) diff);
		closeServer();
	}
}

void CATcpClient::updateClientAlive()
{
	mSocketServer.ulAliveTime = nowSecond();
}

void CATcpClient::setIdleTimeout(int nSeconds)
{
	IDLE_TIMEOUT = nSeconds;
}

void CATcpClient::runIdleTimeout(bool bRun)
{
	if (bRun && (0 < IDLE_TIMEOUT))
	{
		setTimer(IDLE_TIMER, 3, 1, EVENT_FILTER_SOCKET_CLIENT);
	}
	else
	{
		killTimer(IDLE_TIMER);
	}
}
/**========================================================================================================
 *  IPC Message queue callback function.
 *  Receive MSQ message from sendMessage.
 */
void CATcpClient::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	if (callbackReceiveMessage(nEvent, nCommand, nId, nDataLen, pData))
	{
		return;
	}
	unsigned long int ulThreadID;
	unsigned long int ulSocjetFD;

	switch (nCommand)
	{

	case EVENT_COMMAND_SOCKET_CONNECT:
		onServerConnect(nId);
		_log("[CATcpClient] Socket Server Connect FD: %lu", nId);
		updateClientAlive();
		break;
	case EVENT_COMMAND_SOCKET_DISCONNECT: // Server Disconnect
		onServerDisconnect(nId);
		ulThreadID = getThreadID();
		if (ulThreadID)
		{
			threadJoin(ulThreadID);
		}
		_log("[CATcpClient] Socket Server Disconnect FD: %lu", nId);
		break;
	case EVENT_COMMAND_SOCKET_SERVER_COLSE: // Client close Server
		ulThreadID = getThreadID();
		if (ulThreadID)
		{
			threadCancel(ulThreadID);
			threadJoin(ulThreadID);
		}
		break;
	case EVENT_COMMAND_THREAD_EXIT:

		_log("[CATcpClient] Receive Thread Joined, Thread ID: %lu", nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_RECEIVE:

		onReceive(nId, nDataLen, pData);

		break;
	case EVENT_COMMAND_TIMER:
		switch (nId)
		{
		case IDLE_TIMER:
			checkIdle();
			break;
		default:
			onTimer(nId); // overload function
		}
		break;
	case EVENT_COMMAND_SOCKET_TCP_CONNECT_ALIVE:
		updateClientAlive();
		break;
	default:
		_log("[CATcpClient] Unknown message command");
		break;
	}

}
