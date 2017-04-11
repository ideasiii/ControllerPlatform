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
#include "CThread.h"

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
	pthread_mutex_init(&mutex, 0);
}

CObject::~CObject()
{
	pthread_mutex_destroy(&mutex);
}

void CObject::closeMsq()
{
	clearMessage();
	delete messageHandler;
}

int CObject::initMessage(int nKey, const char * szDescript)
{
	int nMsqid;

	if(0 >= nKey)
		nKey = 20150727;
	nMsqid = messageHandler->init(nKey);
	if(0 >= nMsqid)
	{

		szDescript ?
				_log("[Object] %s Register Message Queue Id: %d , Key: %d Fail m>_<m***", szDescript, nMsqid, nKey) :
				_log("[Object] Register Message Queue Id: %d , Key: %d Fail m>_<m***", nMsqid, nKey);
		return -1;
	}

	szDescript ?
			_log("[Object] %s Register Message Queue Id: %d , Key: %d Success ^^Y", szDescript, nMsqid, nKey) :
			_log("[Object] Register Message Queue Id: %d , Key: %d Success ^^Y", nMsqid, nKey);

	return nMsqid;
}

int CObject::run(int lFilter, const char * szDescript)
{
	int nRecv;
	MESSAGE_BUF *msgbuf;

	if(-1 == messageHandler->getMsqid())
	{
		_log("[Object] Invalid msqid, not init msq");
		return -1;
	}

	if(0 >= lFilter)
	{
		_log("[Object] Invalid receive event id");
		return -1;
	}

	msgbuf = new MESSAGE_BUF;
	void * pdata;
	pdata = msgbuf;
	messageHandler->setRecvEvent(lFilter);

	szDescript ?
			_log("[Object] %s Message Receiver Start Run , Event Filter ID:%d ", szDescript, lFilter) :
			_log("[Object] Message Receiver Start Run , Event Filter ID:%d", lFilter);

	while(1)
	{
		memset(msgbuf, 0, sizeof(MESSAGE_BUF));

		nRecv = messageHandler->recvMessage(&pdata);
		if(0 < nRecv)
		{
			onReceiveMessage(msgbuf->lFilter, msgbuf->nCommand, msgbuf->nId, msgbuf->nDataLen, msgbuf->cData);
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

	szDescript ? _log("[Object] %s Message Receiver loop end", szDescript) : _log("[Object] Message Receiver loop end");

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

unsigned long int CObject::createThread(void* (*entry)(void*), void* arg, const char *szDesc)
{
	if(szDesc)
		_log("[CObject] createThread %s", szDesc);
	return _CreateThread(entry, arg);
}

void CObject::threadJoin(unsigned long int thdid)
{
	_ThreadJoin(thdid);
}

void CObject::threadExit()
{
	_ThreadExit();
}

int CObject::threadCancel(unsigned long int thread)
{
	return _ThreadCancel(thread);
}

unsigned long int CObject::getThreadID()
{
	return _GetThreadID();
}

void CObject::mutexLock()
{
	pthread_mutex_lock(&mutex);
}

void CObject::mutexUnlock()
{
	pthread_mutex_unlock(&mutex);
}

