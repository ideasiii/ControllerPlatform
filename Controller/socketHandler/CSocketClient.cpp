/*
 * CSocketClient.cpp
 *
 *  Created on: Sep 6, 2012
 *      Author: jugo
 */

#include "CSocketClient.h"
#include "CThreadHandler.h"
#include "IReceiver.h"
#include "packet.h"
#include "LogHandler.h"

int CSocketClient::m_nInternalEventFilter = 6789;

void *threadMessageReceive(void *argv)
{
	CSocketClient* ss = reinterpret_cast<CSocketClient*>(argv);
	ss->runMessageReceive();
	return NULL;
}

void *threadCMPHandler(void *argv)
{
	int nFD;
	CSocketClient* ss = reinterpret_cast<CSocketClient*>(argv);
	nFD = ss->getSocketfd();
	ss->runCMPHandler(nFD);
	return NULL;
}

void *threadClientHandler(void *argv)
{
	int nFD;
	CSocketClient* ss = reinterpret_cast<CSocketClient*>(argv);
	nFD = ss->m_nClientFD;
	ss->threadUnLock();
	ss->runClientHandler(nFD);
	return NULL;
}

CSocketClient::CSocketClient() :
		CSocket(), threadHandler(new CThreadHandler), mnPacketType(PK_CMP), mnPacketHandle(PK_MSQ), m_nClientFD(-1)
{
	m_nInternalFilter = ++m_nInternalEventFilter;
	externalEvent.init();
}

CSocketClient::~CSocketClient()
{
	delete threadHandler;
}

int CSocketClient::start(int nSocketType, const char* cszAddr, short nPort, int nStyle)
{
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
		if ( SOCK_STREAM == nStyle)
		{
			if (-1 == connectServer())
			{
				socketClose();
				return -1;
			}
		}

		if (-1 != externalEvent.m_nMsgId)
		{
			if (-1 == initMessage(externalEvent.m_nMsgId))
			{
				throwException("socket client create message id fail");
			}
		}
		else
		{
			if (-1 == initMessage(m_nInternalFilter))
			{
				throwException("socket client create message id fail");
			}
		}

		threadHandler->createThread(threadMessageReceive, this);
		switch (mnPacketType)
		{
		case PK_BYTE:
			clientHandler((int) getSocketfd());
			break;
		case PK_CMP:
			cmpHandler((int) getSocketfd());
			break;
		}
		//threadHandler->createThread(threadSocketRecvHandler, this);

		_DBG("[Socket Client] Socket connect success, FD:%d", getSocketfd());
		return getSocketfd();
	}

	return -1;
}

void CSocketClient::threadLock()
{
	threadHandler->threadLock();
}

void CSocketClient::threadUnLock()
{
	threadHandler->threadUnlock();
}

void CSocketClient::clientHandler(int nFD)
{
	this->threadLock();
	this->m_nClientFD = nFD;
	threadHandler->createThread(threadClientHandler, this);
}

void CSocketClient::cmpHandler(int nFD)
{
	this->threadLock();
	this->m_nClientFD = nFD;
	threadHandler->createThread(threadCMPHandler, this);
}

void CSocketClient::stop()
{
	socketClose();
}

void CSocketClient::setPackageReceiver(int nMsgId, int nEventFilter, int nCommand)
{
	externalEvent.m_nMsgId = nMsgId;
	externalEvent.m_nEventFilter = nEventFilter;
	externalEvent.m_nEventRecvCommand = nCommand;
}

void CSocketClient::setClientDisconnectCommand(int nCommand)
{
	externalEvent.m_nEventDisconnect = nCommand;
}

void CSocketClient::runMessageReceive()
{
	run(m_nInternalFilter);
	threadHandler->threadExit();
	threadHandler->threadJoin(threadHandler->getThreadID());
}

int CSocketClient::runCMPHandler(int nSocketFD)
{
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

	struct sockaddr_in *clientSockaddr;
	clientSockaddr = new struct sockaddr_in;

	if (externalEvent.isValid() && -1 != externalEvent.m_nEventConnect)
	{
		_log("send message socket connected");
		sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventConnect, nSocketFD, 0, 0);
	}

	while (1)
	{
		memset(&cmpPacket, 0, sizeof(CMP_PACKET));
		result = socketrecv(nSocketFD, sizeof(CMP_HEADER), &pHeader, clientSockaddr);

		if (sizeof(CMP_HEADER) == result)
		{
			nTotalLen = ntohl(cmpPacket.cmpHeader.command_length);
			nCommand = ntohl(cmpPacket.cmpHeader.command_id);
			nSequence = ntohl(cmpPacket.cmpHeader.sequence_number);
			if ( enquire_link_request == nCommand)
			{
				memset(&cmpHeader, 0, sizeof(CMP_HEADER));
				nCommandResp = generic_nack | nCommand;
				cmpHeader.command_id = htonl(nCommandResp);
				cmpHeader.command_status = htonl( STATUS_ROK);
				cmpHeader.sequence_number = htonl(nSequence);
				cmpHeader.command_length = htonl(sizeof(CMP_HEADER));
				socketSend(nSocketFD, &cmpHeader, sizeof(CMP_HEADER));
				continue;
			}

			nBodyLen = nTotalLen - sizeof(CMP_HEADER);

			if (0 == nBodyLen)
			{
				_log("[Socket Client] Receive CMP:%d", nCommand);
				continue;
			}
			if (0 < nBodyLen)
			{
				result = socketrecv(nSocketFD, nBodyLen, &pBody, 0);
				if (result != nBodyLen)
				{
					if (externalEvent.isValid() && -1 != externalEvent.m_nEventDisconnect)
					{
						sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventDisconnect, nSocketFD, 0, 0);
					}
					socketClose(nSocketFD);
					_log("[Socket Client] socket client close : %d , packet length error: %d != %d", nSocketFD,
							nBodyLen, result);
					break;
				}
			}
		}
		else
		{
			if (externalEvent.isValid() && -1 != externalEvent.m_nEventDisconnect)
			{
				sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventDisconnect, nSocketFD, 0, 0);
			}
			socketClose(nSocketFD);
			_log("[Socket Client] socket client close : %d , packet header length error: %d", nSocketFD, result);
			break;
		}

		if (0 >= result)
		{
			if (externalEvent.isValid() && -1 != externalEvent.m_nEventDisconnect)
			{
				sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventDisconnect, nSocketFD, 0, 0);
			}
			socketClose(nSocketFD);
			break;
		}

		switch (mnPacketHandle)
		{
		case PK_MSQ:
			if (externalEvent.isValid())
			{
				sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventRecvCommand, nSocketFD, nTotalLen,
						&cmpPacket);
			}
			else
			{
				sendMessage(m_nInternalFilter, EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nSocketFD, nTotalLen, &cmpPacket);
			}
			break;
		case PK_ASYNC:
			ClientReceive(nSocketFD, nTotalLen, &cmpPacket);
			break;
		}
	} // while

	sendMessage(m_nInternalFilter, EVENT_COMMAND_THREAD_EXIT, threadHandler->getThreadID(), 0, NULL);

	threadHandler->threadSleep(1);
	threadHandler->threadExit();
}

void CSocketClient::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_THREAD_EXIT:
		threadHandler->threadJoin(nId);
		_log("[Socket Client] Thread Join:%d", (int) nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_RECEIVE:
		_log("[Socket Client] Receive CMP , SocketFD:%d", (int) nId);
		break;
	default:
		_log("[Socket Server] unknow message command");
		break;
	}
}

void CSocketClient::setPacketConf(int nType, int nHandle)
{
	if (0 <= nType && PK_TYPE_SIZE > nType)
		mnPacketType = nType;

	if (0 <= nHandle && PK_HANDLE_SIZE > nHandle)
		mnPacketHandle = nHandle;
}

int CSocketClient::runClientHandler(int nClientFD)
{
	int result;
	char pBuf[BUF_SIZE];
	void* pvBuf = pBuf;
	char szTmp[16];

	/**
	 * clientSockaddr is used for UDP server send packet to client.
	 */
	struct sockaddr_in *clientSockaddr;
	clientSockaddr = new struct sockaddr_in;

	if (externalEvent.isValid() && -1 != externalEvent.m_nEventConnect)
	{
		//_log("send message socket connected");
		sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventConnect, nClientFD, 0, 0);
	}

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
			break;
		}

		if (externalEvent.isValid())
		{
			sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventRecvCommand, nClientFD, result, pBuf);
		}
		else
		{
			sendMessage(m_nInternalFilter, EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nClientFD, result, pBuf);
		}
	}

	delete clientSockaddr;

	sendMessage(m_nInternalFilter, EVENT_COMMAND_THREAD_EXIT, threadHandler->getThreadID(), 0, NULL);

	threadHandler->threadSleep(1);
	threadHandler->threadExit();

	return 0;
}

int CSocketClient::runCMPHandler(int nClientFD)
{
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

	if (externalEvent.isValid() && -1 != externalEvent.m_nEventConnect)
	{
		_log("send message socket connected");
		sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventConnect, nClientFD, 0, 0);
	}

	while (1)
	{

		memset(&cmpPacket, 0, sizeof(cmpPacket));
		result = socketrecv(nClientFD, sizeof(CMP_HEADER), &pHeader, clientSockaddr);

		if (sizeof(CMP_HEADER) == result)
		{
			nTotalLen = ntohl(cmpPacket.cmpHeader.command_length);
			nCommand = ntohl(cmpPacket.cmpHeader.command_id);
			nSequence = ntohl(cmpPacket.cmpHeader.sequence_number);

			memset(&cmpHeader, 0, sizeof(CMP_HEADER));
			nCommandResp = generic_nack | nCommand;
			cmpHeader.command_id = htonl(nCommandResp);
			cmpHeader.command_status = htonl( STATUS_ROK);
			cmpHeader.sequence_number = htonl(nSequence);
			cmpHeader.command_length = htonl(sizeof(CMP_HEADER));

			if ( enquire_link_request == nCommand)
			{
				socketSend(nClientFD, &cmpHeader, sizeof(CMP_HEADER));
				continue;
			}

			nBodyLen = nTotalLen - sizeof(CMP_HEADER);

			if (0 < nBodyLen)
			{
				result = socketrecv(nClientFD, nBodyLen, &pBody, clientSockaddr);
				if (result != nBodyLen)
				{
					socketSend(nClientFD, "Invalid Packet\r\n", strlen("Invalid Packet\r\n"));
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

			if (access_log_request == nCommand)
			{
				socketSend(nClientFD, &cmpHeader, sizeof(CMP_HEADER));
			}
		}
		else if (0 >= result)
		{
			if (externalEvent.isValid() && -1 != externalEvent.m_nEventDisconnect)
			{
				sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventDisconnect, nClientFD, 0, 0);
			}
			socketClose(nClientFD);
			break;
		}
		else
		{
			socketSend(nClientFD, "Control Center: Please use CMP to communicate\r\n",
					strlen("Control Center: Please use CMP to communicate\r\n"));
			_log("[Socket Server] Send Message: Please use CMP to communicate");

			if (externalEvent.isValid() && -1 != externalEvent.m_nEventDisconnect)
			{
				sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventDisconnect, nClientFD, 0, 0);
			}

			socketClose(nClientFD);
			_log("[Socket Server] socket close client: %d , packet header length error: %d", nClientFD, result);
			break;
		}

		switch (mnPacketHandle)
		{
		case PK_MSQ:
			if (externalEvent.isValid())
			{
				sendMessage(externalEvent.m_nEventFilter, externalEvent.m_nEventRecvCommand, nClientFD, nTotalLen,
						&cmpPacket);
			}
			else
			{
				sendMessage(m_nInternalFilter, EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nClientFD, nTotalLen, &cmpPacket);
			}
			break;
		case PK_ASYNC:
			ClientReceive(nClientFD, nTotalLen, &cmpPacket);
			break;
		}

	} // while

	delete clientSockaddr;

	sendMessage(m_nInternalFilter, EVENT_COMMAND_THREAD_EXIT, threadHandler->getThreadID(), 0, NULL);

	threadHandler->threadSleep(1);
	threadHandler->threadExit();

	return 0;
}
