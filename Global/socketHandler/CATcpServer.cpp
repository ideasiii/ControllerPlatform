/*
 * CATcpServer.cpp
 *
 *  Created on: 2017年3月15日
 *      Author: root
 */

#include "CATcpServer.h"
#include "CMessageHandler.h"
#include <time.h>
#include "LogHandler.h"
#include "event.h"
#include "utility.h"

using namespace std;

#define IDLE_TIMER			469107
#define MAX_CLIENT			666
int IDLE_TIMEOUT = 10; // secons

void *threadTcpAccept(void *argv)
{
	CATcpServer* ss = reinterpret_cast<CATcpServer*>(argv);
	ss->runSocketAccept();
	return 0;
}

void *threadCATcpServerMessageReceive(void *argv)
{
	CATcpServer* ss = reinterpret_cast<CATcpServer*>(argv);
	ss->runMessageReceive();
	return 0;
}

void *threadTcpReceive(void *argv)
{
	CATcpServer* ss = reinterpret_cast<CATcpServer*>(argv);
	ss->runTcpReceive();
	return 0;
}

int CATcpServer::start(const char* cszAddr, short nPort)
{
	int nMsgId = -1;
	int nSocketFD;

	mnMsqKey = clock();
	if (-1 == mnMsqKey)
		mnMsqKey = 20150727;

	nMsgId = initMessage(mnMsqKey);

	if (-1 == nMsgId)
	{
		_log("[CATcpServer] Init Message Queue Fail");
		return -1;
	}

	if (-1 == setInetSocket(cszAddr, nPort))
	{
		_log("[CATcpServer] Set INET socket address & port fail");
		return -1;
	}

	nSocketFD = createSocket(AF_INET, SOCK_STREAM);

	if (-1 != nSocketFD)
	{

		if (-1 != socketBind())
		{
			if (-1 == socketListen(BACKLOG))
			{
				perror("socket listen");
				socketClose();
				return -1;
			}
			createThread(threadCATcpServerMessageReceive, this, "CATcpServer Message Receive");
			createThread(threadTcpAccept, this, "CATcpServer Socket Accept Thread");
			_log("[CATcpServer] Create Server Success Socket FD: %lu", nSocketFD);
		}
		else
		{
			socketClose();
		}
	}
	else
		_log("[CATcpServer] Create Server Fail");

	return nSocketFD;
}

void CATcpServer::stop()
{
	socketClose();

	/**
	 * Close Message queue run thread
	 */
	if (0 < munRunThreadId)
	{
		threadCancel(munRunThreadId);
		threadJoin(munRunThreadId);
		munRunThreadId = 0;
		CMessageHandler::closeMsg(CMessageHandler::registerMsq(mnMsqKey));
		_log("[CATcpServer] Message Queue Receive Thread Stop.");
	}

	/**
	 * Close all Client Socket
	 */
	map<unsigned long int, SOCKET_CLIENT>::iterator it;
	for (it = mapClient.begin(); mapClient.end() != it; ++it)
	{
		socketClose(it->first);
		threadCancel(it->second.ulReceiveThreadID);
		threadJoin(it->second.ulReceiveThreadID);
	}
	mapClient.clear();
}

void CATcpServer::closeClient(int nClientFD)
{
	if (mapClient.end() != mapClient.find(nClientFD))
	{
		sendMessage(mnMsqKey, EVENT_COMMAND_SOCKET_CLIENT_COLSE, nClientFD, 0, 0);
	}
}

void CATcpServer::runSocketAccept()
{
	int nChildSocketFD = -1;

	_log("[CATcpServer] Thread runSocketAccept Start");
	while (1)
	{
		nChildSocketFD = socketAccept();

		if (MAX_CLIENT < (mapClient.size() + 1))
		{
			_log("[CATcpServer] Max Client Connect: %d", mapClient.size());
			socketClose(nChildSocketFD);
			sleep(5);
			continue;
		}

		if (-1 != nChildSocketFD)
		{
			_log("[CATcpServer] Socket Accept, Client Socket ID: %d", nChildSocketFD);
			sendMessage(mnMsqKey, EVENT_COMMAND_SOCKET_ACCEPT, nChildSocketFD, 0, NULL);
		}
		else
		{
			_log("socket server accept fail");
			sleep(3);
		}
	}

	_log("[CATcpServer] Thread runSocketAccept End");
	sendMessage(mnMsqKey, EVENT_COMMAND_THREAD_EXIT, getThreadID(), 0, 0);
	threadExit();
}

void CATcpServer::runMessageReceive()
{
	munRunThreadId = getThreadID();
	run(mnMsqKey, "CATcpServer");
	threadExit();
	threadJoin(getThreadID());
	_log("[CATcpServer] runMessageReceive Stop, Thread join");
}

int CATcpServer::getEventId()
{
	return mnMsqKey;
}

void CATcpServer::runTcpReceive()
{
	int result;
	int nSocketFD;

	nSocketFD = getClientSocketFD(getThreadID());
	if (0 >= nSocketFD)
	{
		_log("[CATcpServer] runTcpReceive Fail, Invalid Socket FD");
		sendMessage(mnMsqKey, EVENT_COMMAND_THREAD_EXIT, getThreadID(), 0, 0);
		threadExit();
		return;
	}

	sendMessage(mnMsqKey, EVENT_COMMAND_SOCKET_CONNECT, nSocketFD, 0, 0);

	while (1)
	{
		updateClientAlive(nSocketFD);
		if (0 >= onTcpReceive(nSocketFD))
			break;
	}

	sendMessage(mnMsqKey, EVENT_COMMAND_SOCKET_DISCONNECT, nSocketFD, 0, 0);
	threadExit();
}

int CATcpServer::onTcpReceive(unsigned long int nSocketFD)
{
	int result;
	char pBuf[BUF_SIZE];
	void* pvBuf = pBuf;
	memset(pBuf, 0, sizeof(pBuf));
	result = socketrecv(nSocketFD, &pvBuf, 0);
	_log("[CATcpServer] onTcpReceive Result: %d Socket[%d]", result, nSocketFD);

	if (0 < result)
	{
		sendMessage(mnMsqKey, EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nSocketFD, result, pBuf);
	}
	return result;
}

unsigned long int CATcpServer::getClientSocketFD(unsigned long int unThreadId)
{
	map<unsigned long int, SOCKET_CLIENT>::iterator it;

	for (it = mapClient.begin(); mapClient.end() != it; ++it)
	{
		if (it->second.ulReceiveThreadID == unThreadId)
		{
			return it->first;
		}
	}
	return 0;
}

unsigned long int CATcpServer::getClientThreadID(unsigned long int unSocketFD)
{
	if (mapClient.find(unSocketFD) != mapClient.end())
	{
		return mapClient[unSocketFD].ulReceiveThreadID;
	}
	return 0;
}

void CATcpServer::setIdleTimeout(int nSeconds)
{
	IDLE_TIMEOUT = nSeconds;
}

void CATcpServer::runIdleTimeout(bool bRun)
{
	if (bRun && (0 < IDLE_TIMEOUT))
		setTimer(IDLE_TIMER, 3, 1, mnMsqKey);
	else
		killTimer(IDLE_TIMER);
}

void CATcpServer::checkIdle()
{
	map<unsigned long int, SOCKET_CLIENT>::iterator it;
	double diff;
	for (it = mapClient.begin(); mapClient.end() != it; ++it)
	{
		diff = difftime(nowSecond(), it->second.ulAliveTime);
		if (IDLE_TIMEOUT < (int) diff)
		{
			_log("[CATcpServer] Socket Client: %d idle: %d seconds", it->first, (int) diff);
			closeClient(it->first);
		}
	}
}

void CATcpServer::eraseClient(unsigned long int ulSocketFD)
{
	if (mapClient.find(ulSocketFD) != mapClient.end())
	{
		mapClient.erase(ulSocketFD);
	}
}

void CATcpServer::updateClientAlive(unsigned long int ulSocketFD)
{
	if (mapClient.find(ulSocketFD) != mapClient.end())
	{
		mapClient[ulSocketFD].ulAliveTime = nowSecond();
	}
}
/**========================================================================================================
 *  IPC Message queue callback function.
 *  Receive MSQ message from sendMessage.
 */
void CATcpServer::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	mutexLock();
	unsigned long int ulThreadID;
	unsigned long int ulSocjetFD;

	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_ACCEPT:
		mapClient[nId].ulReceiveThreadID = createThread(threadTcpReceive, this);
		if (0 >= mapClient[nId].ulReceiveThreadID)
		{
			eraseClient(nId);
			socketClose(nId);
		}
		break;
	case EVENT_COMMAND_SOCKET_CONNECT:
		_log("[CATcpServer] Socket Client Connect FD: %lu", nId);
		updateClientAlive(nId);
		break;
	case EVENT_COMMAND_SOCKET_DISCONNECT: // Client Disconnect
		ulThreadID = getClientThreadID(nId);
		if (ulThreadID)
			threadJoin(ulThreadID);
		eraseClient(nId);
		socketClose(nId);
		_log("[CATcpServer] Socket Client Disconnect FD: %lu", nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_COLSE: // Server close Client
		ulThreadID = getClientThreadID(nId);
		if (ulThreadID)
		{
			threadCancel(ulThreadID);
			threadJoin(ulThreadID);
		}
		eraseClient(nId);
		socketClose(nId);
		break;
	case EVENT_COMMAND_THREAD_EXIT:
		threadJoin(nId);
		ulSocjetFD = getClientSocketFD(nId);
		if (ulSocjetFD)
			eraseClient(ulSocjetFD);
		_log("[CATcpServer] Receive Thread Joined, Thread ID: %lu", nId);
		break;
	case EVENT_COMMAND_SOCKET_SERVER_RECEIVE:
		updateClientAlive(nId);
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
	default:
		_log("[CATcpServer] Unknow message command");
		break;
	}
	mutexUnlock();
}
