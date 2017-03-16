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

using namespace std;

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
	if(-1 == mnMsqKey)
		mnMsqKey = 20150727;

	nMsgId = initMessage(mnMsqKey);

	if(-1 == nMsgId)
	{
		_log("[CATcpServer] Init Message Queue Fail");
		return -1;
	}

	if(-1 == setInetSocket(cszAddr, nPort))
	{
		_log("[CATcpServer] Set INET socket address & port fail");
		return -1;
	}

	createThread(threadCATcpServerMessageReceive, this);
	nSocketFD = createSocket(AF_INET, SOCK_STREAM);

	if(-1 != nSocketFD)
	{
		if(-1 != socketBind())
		{
			if(-1 == socketListen(BACKLOG))
			{
				perror("socket listen");
				socketClose();
				return -1;
			}
			createThread(threadTcpAccept, this);
		}
		else
		{
			socketClose();
		}
	}

	_log("[CATcpServer] Create Server Socket FD: %lu", nSocketFD);
	return nSocketFD;
}

void CATcpServer::stop()
{
	if(0 < munRunThreadId)
	{
		threadCancel(munRunThreadId);
		threadJoin(munRunThreadId);
		munRunThreadId = 0;
		CMessageHandler::closeMsg(CMessageHandler::registerMsq(mnMsqKey));
		_log("[CATcpServer] Message Queue Receive Thread Stop.");
	}
}

void CATcpServer::runSocketAccept()
{
	int nChildSocketFD = -1;

	_log("[CATcpServer] Thread runSocketAccept Start");
	while(1)
	{
		nChildSocketFD = socketAccept();

		if(-1 != nChildSocketFD)
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

	sendMessage(mnMsqKey, EVENT_COMMAND_THREAD_EXIT, getThreadID(), 0, 0);
	threadExit();
	_log("[CATcpServer] Thread runSocketAccept End");
}

void CATcpServer::runMessageReceive()
{
	munRunThreadId = getThreadID();
	run(mnMsqKey, "CATcpServer");
	threadExit();
	threadJoin(getThreadID());
	_log("[CATcpServer] runMessageReceive Stop, Thread join");
}

void CATcpServer::runTcpReceive()
{
	int result;
	int nSocketFD;
	char pBuf[BUF_SIZE];
	void* pvBuf = pBuf;

	nSocketFD = getClientSocketFD(getThreadID());
	if(0 >= nSocketFD)
	{
		_log("[CATcpServer] runTcpReceive Fail, Invalid Socket FD");
		sendMessage(mnMsqKey, EVENT_COMMAND_THREAD_EXIT, getThreadID(), 0, 0);
		threadExit();
		return;
	}

	struct sockaddr_in *clientSockaddr;
	clientSockaddr = new struct sockaddr_in;

	sendMessage(mnMsqKey, EVENT_COMMAND_SOCKET_CONNECT, nSocketFD, 0, 0);

	while(1)
	{
		memset(pBuf, 0, sizeof(pBuf));
		result = socketrecv(nSocketFD, &pvBuf, clientSockaddr);

		if(0 >= result)
		{
			sendMessage(mnMsqKey, EVENT_COMMAND_SOCKET_DISCONNECT, nSocketFD, 0, 0);
			socketClose(nSocketFD);
			break;
		}
		sendMessage(mnMsqKey, EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nSocketFD, result, pBuf);
	}
	mapClientThread.erase(nSocketFD);
	delete clientSockaddr;
	sendMessage(mnMsqKey, EVENT_COMMAND_THREAD_EXIT, getThreadID(), 0, NULL);
	threadExit();
}

unsigned long int CATcpServer::getClientSocketFD(unsigned long int unThreadId)
{
	for(map<unsigned long int, unsigned long int>::const_iterator it = mapClientThread.begin();
			it != mapClientThread.end(); ++it)
	{
		if(it->second == unThreadId)
		{
			return it->first;
		}
	}
	return 0;
}

/**========================================================================================================
 *  IPC Message queue callback function.
 *  Receive MSQ message from sendMessage.
 */
void CATcpServer::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch(nCommand)
	{
	case EVENT_COMMAND_SOCKET_ACCEPT:
		mapClientThread[nId] = createThread(threadTcpReceive, this);
		if(0 >= mapClientThread[nId])
		{
			mapClientThread.erase(nId);
			socketClose(nId);
		}
		break;
	case EVENT_COMMAND_SOCKET_CONNECT:
		_log("[CATcpServer] Socket Client Connect FD: %lu", nId);
		break;
	case EVENT_COMMAND_SOCKET_DISCONNECT:
		mapClientThread.erase(nId);
		_log("[CATcpServer] Socket Client Disconnect FD: %lu", nId);
		break;
	case EVENT_COMMAND_THREAD_EXIT:
		threadJoin(nId);
		_log("[CATcpServer] Receive Thread Joined, Thread ID: %lu", nId);
		break;
	case EVENT_COMMAND_SOCKET_SERVER_RECEIVE:
		_log("[CATcpServer] Receive Package , Socket FD: %lu", nId);
		break;
	case EVENT_COMMAND_TIMER:
		_log("[CATcpServer] On Timer , ID: %lu", nId);
		break;
	default:
		_log("[CATcpServer] Unknow message command");
		break;
	}
}
