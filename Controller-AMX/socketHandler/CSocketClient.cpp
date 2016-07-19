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

void *threadSocketRecvHandler(void *argv)
{
	int nFD;
	CSocketClient* ss = reinterpret_cast<CSocketClient*>(argv);
	nFD = ss->getSocketfd();
	ss->runSMSSocketReceive(nFD);
	return NULL;
}

CSocketClient::CSocketClient() :
		CSocket(), threadHandler(new CThreadHandler)
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
		threadHandler->createThread(threadSocketRecvHandler, this);

		_DBG("[Socket Client] Socket connect success, FD:%d", getSocketfd());
		return getSocketfd();
	}

	return -1;
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

void CSocketClient::runSMSSocketReceive(int nSocketFD)
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
					_log("[Socket Client] socket client close : %d , packet length error: %d != %d", nSocketFD,
							nBodyLen, result);
					break;
				}
			}
		}
		else
		{
			socketClose(nSocketFD);
			_log("[Socket Client] socket client close : %d , packet header length error: %d", nSocketFD, result);
			break;
		}

		if (0 >= result)
		{
			socketClose(nSocketFD);
			break;
		}

	} // while

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
	case EVENT_COMMAND_SOCKET_RECEIVE:
		_log("[Socket Client] Receive CMP , SocketFD:%d", (int) nId);
		break;
	default:
		_log("[Socket Server] unknow message command");
		break;
	}
}