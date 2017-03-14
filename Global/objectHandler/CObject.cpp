/*
 * CObject.cpp
 *
 *  Created on: Sep 18, 2012
 *      Author: jugo
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include "CObject.h"
#include "CMessageHandler.h"
#include "common.h"
#include "memory.h"
#include "LogHandler.h"
#include "CTimer.h"
#include "event.h"

CObject * object = 0;

void _onTimer(int nId)
{
	if(object)
	{
		object->_OnTimer(nId);
	}
}

CObject::CObject() :
		messageHandler(new CMessageHandler), mnTimerEventId(-1)
{

}

CObject::~CObject()
{
	//clearMessage();
	//delete messageHandler;
}

int CObject::initMessage(int nKey)
{
	int nMsqid;

	nMsqid = messageHandler->init(nKey);
	if(0 >= nMsqid)
	{
		//throwException("Create message queue fail");
		_log("[Object] Create Message Queue Id: %d , Key: %d Fail m>_<m***", nMsqid, nKey);
		return FALSE;
	}
	_log("[Object] Create Message Queue Id: %d , Key: %d Success ^^Y", nMsqid, nKey);
	return TRUE;
}

int CObject::run(int nRecvEvent, const char * szDescript)
{
	int nRecv;
	MESSAGE_BUF *msgbuf;

	if(-1 == messageHandler->getMsqid())
	{
		_log("[Object] Invalid msqid, not init msq");
		return -1;
	}

	if(0 >= nRecvEvent)
	{
		_log("[Object] Invalid receive event id");
		return -1;
	}

	msgbuf = new MESSAGE_BUF;
	void * pdata;
	pdata = msgbuf;
	messageHandler->setRecvEvent(nRecvEvent);

	if(szDescript)
		_log("[Object] %s Message Service Start Run , Event ID:%d ", szDescript, nRecvEvent);
	else
		_log("[Object] Message Service Start Run , Event ID:%d", nRecvEvent);

	while(1)
	{
		memset(msgbuf, 0, sizeof(MESSAGE_BUF));

		nRecv = messageHandler->recvMessage(&pdata);
		if(0 < nRecv)
		{
			onReceiveMessage(msgbuf->mtype, msgbuf->nCommand, msgbuf->nId, msgbuf->nDataLen, msgbuf->cData);
		}
		else if(-2 == nRecv)
		{
			/**
			 * get SIGINT
			 */
			break;
		}
		else
		{
			sleep(5);
		}
	}

	delete msgbuf;

	if(szDescript)
		_log("[Object] %s Message loop end", szDescript);
	else
		_log("[Object] Message loop end");
	return 0;
}

int CObject::sendMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	return messageHandler->sendMessage(nEvent, nCommand, nId, nDataLen, pData);
}

void CObject::clearMessage()
{
	messageHandler->close();
}

timer_t CObject::setTimer(int nId, int nSecStart, int nInterSec, int nEvent)
{
	mnTimerEventId = nEvent;
	object = this;
	return _SetTimer(nId, nSecStart, nInterSec, _onTimer);
}

void CObject::killTimer(int nId)
{
	_KillTimer(nId);
}

void CObject::_OnTimer(int nId)
{
	if(-1 != mnTimerEventId)
	{
		messageHandler->sendMessage(mnTimerEventId, EVENT_COMMAND_TIMER, nId, 0, 0);
	}
	onTimer(nId);
}

