/*
 * CMessageHandler.cpp
 *
 *  Created on: Dec 02, 2014
 *      Author: jugo
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "CMessageHandler.h"
#include "common.h"
#include "LogHandler.h"

CMessageHandler::CMessageHandler() :
		msqid(-1), m_nEvent(-1)
{
	buf_length = sizeof(struct MESSAGE_BUF) - sizeof(long);
}

CMessageHandler::~CMessageHandler()
{

}

void CMessageHandler::close()
{
	if (-1 == msqid)
	{
		_log("[Message] close message fail, invalid meqid");
		return;
	}
	if (msgctl(msqid, IPC_RMID, NULL) == -1)
	{
		perror("msgctl");
	}

	_log("[Message] message queue close");
}

int CMessageHandler::init(const long lkey)
{
	int nMsqid;
	int msgflg = IPC_CREAT | 0666;

	if (0 >= lkey)
	{
		nMsqid = -1;
	}
	else
	{
		nMsqid = msgget(lkey, msgflg);

		if (0 >= nMsqid)
		{
			_log("[Message] message queue init fail, msqid=%d error=%s errorno=%d", nMsqid, strerror(errno),
			errno);
		}
		else
		{
			/**
			 * config msq
			 */
			struct msqid_ds ds;

			memset(&ds, 0, sizeof(struct msqid_ds));
			if (msgctl(nMsqid, IPC_STAT, &ds))
			{
				_log("[Message] message queue control fail, msqid=%d error=%s errorno=%d", nMsqid, strerror(errno),
				errno);
			}
			else
			{
				//	_DBG("[Message] Queue size = %lu", ds.msg_qbytes);
				ds.msg_qbytes = 1024 * 1024 * 8;
				if (msgctl(nMsqid, IPC_SET, &ds))
				{
					_log("[Message] message queue control fail, msqid=%d error=%s errorno=%d", nMsqid, strerror(errno),
					errno);
				}
			}
		}
	}

	setMsqid(nMsqid);

	return nMsqid;

}

int CMessageHandler::sendMessage(int nType, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	int nRet;
	MESSAGE_BUF *pBuf;

	pBuf = new MESSAGE_BUF;

	pBuf->mtype = nType;
	pBuf->nCommand = nCommand;
	pBuf->nId = nId;

	memset(pBuf->cData, 0, sizeof(pBuf->cData));
	if ( NULL != pData && 0 < nDataLen)
	{
		memcpy(pBuf->cData, pData, nDataLen);
		pBuf->nDataLen = nDataLen;
	}

	if (-1 == msgsnd(getMsqid(), pBuf, getBufLength(), /*IPC_NOWAIT*/0))
	{
		_log("[Message] message queue send fail, msqid=%d error=%s errorno=%d", getMsqid(), strerror(errno),
		errno);
		nRet = -1;
	}
	else
	{
		nRet = getBufLength();
	}

	delete pBuf;

	return nRet;
}

int CMessageHandler::recvMessage(void **pbuf)
{
	ssize_t recvSize = 0;

	if ( NULL == *pbuf)
		return -1;

	if (-1 == getRecvEvent())
	{
		_log("[Message] message queue receive fail, msqid=%d invalid event", getMsqid());
		return -1;
	}

	recvSize = msgrcv(getMsqid(), *pbuf, getBufLength(), getRecvEvent(), 0);

	if (0 > recvSize)
	{
		if ( errno == EINTR)
		{
			_log("[Message] message queue receive fail get EINTR, msqid=%d error=%s errorno=%d", getMsqid(),
					strerror(errno),
					errno);
			return -2;
		}
		return -1;
	}

	return recvSize;
}

void CMessageHandler::setRecvEvent(int nEvent)
{
	m_nEvent = nEvent;
}

int CMessageHandler::getRecvEvent() const
{
	return m_nEvent;
}

void CMessageHandler::setMsqid(int nId)
{
	msqid = nId;
}

int CMessageHandler::getMsqid() const
{
	return msqid;
}

int CMessageHandler::getBufLength() const
{
	return buf_length;
}

//************* Static Function **************//

int CMessageHandler::registerMsq(const long lkey)
{
	int nMsqid;
	int msgflg = IPC_CREAT | 0666;

	if (0 >= lkey)
	{
		nMsqid = -1;
	}
	else
	{
		nMsqid = msgget(lkey, msgflg);

		if (0 >= nMsqid && 17 != errno)
		{
			_log("[Message] message queue init fail, msqid=%d error=%s errorno=%d", nMsqid, strerror(errno),
			errno);
		}
	}

	return nMsqid;
}

void CMessageHandler::closeMsg(const int nMsqId)
{
	if (0 >= nMsqId)
	{
		_log("[Message] Error, close message fail, invalid meqid : %d", nMsqId);
		return;
	}
	if (-1 == msgctl(nMsqId, IPC_RMID, NULL))
		_log("[Message] Error, close message fail, message queue control fail");
	else
		_log("[Message] message queue close, id: %d", nMsqId);
}
