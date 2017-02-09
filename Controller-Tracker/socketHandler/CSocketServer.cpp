/*
 * CSocketServer.cpp
 *
 *  Created on: 2015年10月19日
 *      Author: Louis Ju
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <typeinfo>
#include "CSocketServer.h"
#include "CThreadHandler.h"
#include "common.h"
#include "CDataHandler.cpp"
#include "packet.h"
#include "IReceiver.h"
#include "LogHandler.h"
void *threadSocketMessageReceive(void *argv);
void *threadSocketAccept(void *argv);
void *threadClientHandler(void *argv);
void *threadSMSHandler(void *argv);

int CSocketServer::m_nInternalEventFilter = 7000;

CSocketServer::CSocketServer() :
		CSocket(), m_nClientFD(-1), threadHandler(new CThreadHandler), udpClientData(0)
{
	m_nInternalFilter = ++m_nInternalEventFilter;
	externalEvent.init();
}

CSocketServer::~CSocketServer()
{
	delete threadHandler;
	if (udpClientData)
	{
		delete udpClientData;
	}
}

int CSocketServer::start(int nSocketType, const char* cszAddr, short nPort, int nStyle)
{
	int nMsgId = -1;

	if (-1 != externalEvent.m_nMsgId)
	{
		nMsgId = initMessage(externalEvent.m_nMsgId);
	}
	else
	{
		nMsgId = initMessage(m_nInternalFilter);
	}

	if (-1 == nMsgId)
	{
		throwException("socket server create message id fail");
		return -1;
	}

	threadHandler->createThread(threadSocketMessageReceive, this);

	if ( AF_UNIX == nSocketType)
	{
		setDomainSocketPath(cszAddr);
	}
	else if ( AF_INET == nSocketType)
	{
		if (-1 == setInetSocket(cszAddr, nPort))
		{
			_DBG("set INET socket address & port fail");
			return -1;
		}
	}

	if (-1 != createSocket(nSocketType, nStyle))
	{
		if (-1 != socketBind())
		{
			if ( SOCK_STREAM == nStyle)
			{
				if (-1 == socketListen( BACKLOG))
				{
					perror("socket listen");
					socketClose();
					return -1;
				}

				threadHandler->createThread(threadSocketAccept, this);
			}
			else if ( SOCK_DGRAM == nStyle)
			{
				if (udpClientData)
					delete udpClientData;
				udpClientData = new CDataHandler<struct sockaddr_in>;
				clientHandler(getSocketfd());
			}
			return 0;
		}
		else
		{
			socketClose();
		}
	}

	return -1;
}

void CSocketServer::stop()
{
	socketClose();
}

void CSocketServer::clientHandler(int nFD)
{
	this->threadLock();
	this->m_nClientFD = nFD;
	threadHandler->createThread(threadClientHandler, this);
}

void CSocketServer::smsHandler(int nFD)
{
	this->threadLock();
	this->m_nClientFD = nFD;
	threadHandler->createThread(threadSMSHandler, this);
}

void *threadClientHandler(void *argv)
{
	int nFD;
	CSocketServer* ss = reinterpret_cast<CSocketServer*>(argv);
	nFD = ss->m_nClientFD;
	ss->threadUnLock();
	ss->runClientHandler(nFD);
	return NULL;
}

void *threadSMSHandler(void *argv)
{
	int nFD;
	CSocketServer* ss = reinterpret_cast<CSocketServer*>(argv);
	nFD = ss->m_nClientFD;
	ss->threadUnLock();
	ss->runSMSHandler(nFD);
	return NULL;
}

void CSocketServer::threadLock()
{
	threadHandler->threadLock();
}

void CSocketServer::threadUnLock()
{
	threadHandler->threadUnlock();
}

int CSocketServer::runClientHandler(int nClientFD)
{
	int nFD;
	int result;
	char pBuf[BUF_SIZE];
	void* pvBuf = pBuf;
	char szTmp[16];

	/**
	 * clientSockaddr is used for UDP server send packet to client.
	 */
	struct sockaddr_in *clientSockaddr;
	clientSockaddr = new struct sockaddr_in;

	while (1)
	{
		memset(pBuf, 0, sizeof(pBuf));
		result = socketrecv(nClientFD, &pvBuf, clientSockaddr);

		if (0 >= result)
		{
			if (externalEvent.isValid() && -1 != externalEvent.m_nEventDisconnect)
			{
				sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventDisconnect, nClientFD, 0, 0);
			}
			socketClose(nClientFD);
			_DBG("[Socket Server] socket close client: %d", nClientFD);
			break;
		}

		if (nClientFD == getSocketfd())
		{
			/**
			 * UDP server receive packet
			 */
			nFD = ntohs(clientSockaddr->sin_port);
			memset(szTmp, 0, sizeof(szTmp));
			sprintf(szTmp, "%d", nFD);
			udpClientData->setData(szTmp, *clientSockaddr);
		}
		else
		{
			nFD = nClientFD;
		}

		if (externalEvent.isValid())
		{
			//	_DBG("[Socket Server] Send Message : FD=%d len=%d", nFD, result);
			sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventRecvCommand, nFD, result, pBuf);
		}
		else
		{
			sendMessage(m_nInternalFilter, EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nFD, result, pBuf);
		}
	}

	delete clientSockaddr;

	sendMessage(m_nInternalFilter, EVENT_COMMAND_THREAD_EXIT, threadHandler->getThreadID(), 0, NULL);

	threadHandler->threadSleep(1);
	threadHandler->threadExit();

	return 0;
}

int CSocketServer::runSMSHandler(int nClientFD)
{
	int nFD;
	int result = 0;
	char szTmp[16];
	int nTotalLen = 0;
	int nBodyLen = 0;
	int nCommand = generic_nack;
	int nSequence = 0;

	CMP_PACKET cmpPacket;
	void* pHeader = &cmpPacket.cmpHeader;
	void* pBody = &cmpPacket.cmpBody;

	CMP_HEADER cmpHeader;
	void *pHeaderResp = &cmpHeader;
	int nCommandResp;

	/**
	 * clientSockaddr is used for UDP server send packet to client.
	 */
	struct sockaddr_in *clientSockaddr;
	clientSockaddr = new struct sockaddr_in;

	//********get client address START  *********************************
	bool getCLientIP = false;
	char ipstrp[INET6_ADDRSTRLEN];
	int port;
		socklen_t sockaddrlen = sizeof(sockaddr_in);
	int ret = getpeername(nClientFD,(struct sockaddr *)clientSockaddr, &sockaddrlen);
	if(ret != 0)
	{
		_log("[CSocketClient]getPeerName ERROR");
	}
	else
	{
		inet_ntop(AF_INET, &clientSockaddr->sin_addr,ipstrp,sizeof(ipstrp));
		_log("[CSocketClient]Client Connect START: Socket Client FD: %d Connected From %s",nClientFD,ipstrp);
		getCLientIP = true;
	}
	//********get client address END *********************************
	while (1)
	{
		memset(&cmpPacket, 0, sizeof(cmpPacket));
		result = socketrecv(nClientFD, sizeof(CMP_HEADER), &pHeader, clientSockaddr);

		if (sizeof(CMP_HEADER) == result)
		{
			nTotalLen = ntohl(cmpPacket.cmpHeader.command_length);
			nCommand = ntohl(cmpPacket.cmpHeader.command_id);
			nSequence = ntohl(cmpPacket.cmpHeader.sequence_number);
			if ( enquire_link_request == nCommand)
			{
				printf("*********enquire_link_request******\n");
				memset(&cmpHeader, 0, sizeof(CMP_HEADER));
				nCommandResp = generic_nack | nCommand;
				cmpHeader.command_id = htonl(nCommandResp);
				cmpHeader.command_status = htonl( STATUS_ROK);
				cmpHeader.sequence_number = htonl(nSequence);
				cmpHeader.command_length = htonl(sizeof(CMP_HEADER));
				socketSend(nClientFD, &cmpHeader, sizeof(CMP_HEADER));
				_DBG("[Socket Server] Send Enquire Link Response Sequence:%d Socket FD:%d", nSequence, nClientFD);
				continue;
			}

			nBodyLen = nTotalLen - sizeof(CMP_HEADER);

			if (0 < nBodyLen)
			{
				result = socketrecv(nClientFD, nBodyLen, &pBody, clientSockaddr);
				if (result != nBodyLen)
				{
					if (externalEvent.isValid() && -1 != externalEvent.m_nEventDisconnect)
					{
						sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventDisconnect, nClientFD, 0, 0);
					}
					socketClose(nClientFD);
					_log("[Socket Server] socket close client: %d , packet length error: %d != %d", nClientFD, nBodyLen,
							result);
					break;
				}
			}
		}
		else
		{
			if (externalEvent.isValid() && -1 != externalEvent.m_nEventDisconnect)
			{
				sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventDisconnect, nClientFD, 0, 0);
			}
			socketClose(nClientFD);
			_log("[Socket Server] socket close client: %d , packet header length error: %d", nClientFD, result);
			break;
		}

		if (0 >= result)
		{
			if (externalEvent.isValid() && -1 != externalEvent.m_nEventDisconnect)
			{
				sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventDisconnect, nClientFD, 0, 0);
			}
			socketClose(nClientFD);
			_log("[Socket Server] socket close client: %d", nClientFD);
			break;
		}
		if ( access_log_request == nCommand)
		{
			memset(&cmpHeader, 0, sizeof(CMP_HEADER));
			nCommandResp = generic_nack | nCommand;
			cmpHeader.command_id = htonl(nCommandResp);
			cmpHeader.command_status = htonl( STATUS_ROK);
			cmpHeader.sequence_number = htonl(nSequence);
			cmpHeader.command_length = htonl(sizeof(CMP_HEADER));
			socketSend(nClientFD, &cmpHeader, sizeof(CMP_HEADER));
			_DBG("[Socket Server] Send Access Log Response Sequence:%d Socket FD:%d", nSequence, nClientFD);
		}

		if (nClientFD == getSocketfd())
		{
			/**
			 * UDP server receive packet,record client information
			 */
			nFD = ntohs(clientSockaddr->sin_port);
			memset(szTmp, 0, sizeof(szTmp));
			sprintf(szTmp, "%d", nFD);
			udpClientData->setData(szTmp, *clientSockaddr);
		}
		else
		{
			nFD = nClientFD;
		}

		if (externalEvent.isValid())
		{
			//	_DBG("[Socket Server] Send Message : FD=%d len=%d", nFD, result);
			//use IPC message queue
			sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventRecvCommand, nFD, nTotalLen, &cmpPacket);

			//use thread, not using IPC message queue
			//ServerReceive(nFD, nTotalLen, &cmpPacket);
		}
		else
		{
			sendMessage(m_nInternalFilter, EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nFD, nTotalLen, &cmpPacket);
		}
	} // while

	if(getCLientIP == true)
	{
		_log("[CSocketServer]Client Connect END: Socket Client FD: %d Connected From %s",nClientFD,ipstrp);
	}
	delete clientSockaddr;

	sendMessage(m_nInternalFilter, EVENT_COMMAND_THREAD_EXIT, threadHandler->getThreadID(), 0, NULL);

	threadHandler->threadSleep(1);
	threadHandler->threadExit();

	return 0;
}

void *threadSocketAccept(void *argv)
{
	CSocketServer* ss = reinterpret_cast<CSocketServer*>(argv);
	ss->runSocketAccept();
	return NULL;
}

void CSocketServer::runSocketAccept()
{
	int nChildSocketFD = -1;

	while (1)
	{
		nChildSocketFD = socketAccept();

		if (-1 != nChildSocketFD)
		{
			sendMessage(m_nInternalFilter, EVENT_COMMAND_SOCKET_ACCEPT, nChildSocketFD, 0, NULL);
			if (externalEvent.isValid() && -1 != externalEvent.m_nEventConnect)
			{
				sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventConnect, nChildSocketFD, 0, 0);
			}
		}
		else
		{
			_DBG("socket server accept fail");
			sleep(3);
		}
	}

	sendMessage(m_nInternalFilter, EVENT_COMMAND_THREAD_EXIT, threadHandler->getThreadID(), 0, NULL);
	threadHandler->threadExit();
}

void *threadSocketMessageReceive(void *argv)
{
	CSocketServer* ss = reinterpret_cast<CSocketServer*>(argv);
	ss->runMessageReceive();
	return NULL;
}

void CSocketServer::runMessageReceive()
{
	run(m_nInternalFilter);
	threadHandler->threadExit();
	threadHandler->threadJoin(threadHandler->getThreadID());
}

void CSocketServer::setPackageReceiver(int nMsgId, int nEventFilter, int nCommand)
{
	externalEvent.m_nMsgId = nMsgId;
	externalEvent.m_nEventFilter = nEventFilter;
	externalEvent.m_nEventRecvCommand = nCommand;
}

void CSocketServer::setClientConnectCommand(int nCommand)
{
	externalEvent.m_nEventConnect = nCommand;
}

void CSocketServer::setClientDisconnectCommand(int nCommand)
{
	externalEvent.m_nEventDisconnect = nCommand;
}

void CSocketServer::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_ACCEPT:
		smsHandler((int) nId);
		//clientHandler((int) nId);
		break;
	case EVENT_COMMAND_THREAD_EXIT:
		threadHandler->threadJoin(nId);
		break;
	case EVENT_COMMAND_SOCKET_SERVER_RECEIVE:
		break;
	default:
		_DBG("[Socket Server] unknow message command");
		break;
	}
}

int CSocketServer::getInternalEventFilter() const
{
	return m_nInternalFilter;
}

int CSocketServer::sendtoUDPClient(int nClientId, const void* pBuf, int nBufLen)
{
	int nSend = -1;
	char szId[16];

	memset(szId, 0, sizeof(szId));
	sprintf(szId, "%d", nClientId);
	if (udpClientData && udpClientData->isValidKey(szId))
	{
		nSend = socketSend((*udpClientData)[szId], pBuf, nBufLen);
	}

	return nSend;
}

void CSocketServer::eraseUDPCliefnt(int nClientId)
{
	char szId[16];

	memset(szId, 0, sizeof(szId));
	sprintf(szId, "%d", nClientId);
	if (udpClientData && udpClientData->isValidKey(szId))
	{
		udpClientData->erase(szId);
	}

	_DBG("[Socket Server] UDP client %d in queue", udpClientData->size());
}

