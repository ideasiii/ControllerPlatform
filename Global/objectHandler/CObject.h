/*
 * CObject.cpp
 *
 *  Created on: 2014年12月2日
 *      Author: jugo
 */

#pragma once

#include <stdio.h>
#include <sys/types.h>
#include "LogHandler.h"

class CMessageHandler;

struct EVENT_EXTERNAL
{
	int m_nMsgId;
	int m_nEventFilter;
	int m_nEventRecvCommand;
	int m_nEventDisconnect;
	int m_nEventConnect;
	void init()
	{
		m_nMsgId = -1;
		m_nEventFilter = -1;
		m_nEventRecvCommand = -1;
		m_nEventDisconnect = -1;
		m_nEventConnect = -1;
	}
	bool isValid()
	{
		if (-1 != m_nMsgId && -1 != m_nEventFilter && -1 != m_nEventRecvCommand)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

class CObject
{
public:
	explicit CObject();
	virtual ~CObject();
	void clearMessage();
	int sendMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
	void _OnTimer(int nId);
	int initMessage(int nKey);
	int run(int nRecvEvent, const char * szDescript = 0);
	timer_t setTimer(int nId, int nSecStart, int nInterSec, int nEvent = -1);
	void killTimer(int nId);
	unsigned long int createThread(void* (*entry)(void*), void* arg, const char *szDesc = 0);
	void threadJoin(unsigned long int thdid);
	void threadExit();
	int threadCancel(unsigned long int thread);
	void mutexLock();
	void mutexUnlock();
	unsigned long int getThreadID();
	void closeMsq();

protected:
	// virtual function, child must overload
	virtual void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData) = 0;
	virtual void onTimer(int nId)
	{
	}
	;

private:
	CMessageHandler *messageHandler;
	int mnTimerEventId;
	pthread_mutex_t mutex;
};
