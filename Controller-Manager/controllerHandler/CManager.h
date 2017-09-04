/*
 * CManager.h
 *
 *  Created on: 2017年8月18日
 *      Author: root
 */

#pragma once

#include "CCmpServer.h"

class CProcessManager;

typedef struct _MONITOR
{
	int nThread_Max;
	_MONITOR()
	{
		nThread_Max = 30;
	}
} MONITOR;

class CManager: public CCmpServer
{
public:
	explicit CManager(CObject *object);
	virtual ~CManager();
	void checkProcess();
	void psInstanceDump(int pid);
	void setThreadMax(int nMax);

private:
	CObject *mpController;
	CProcessManager *processmanager;

private:
	MONITOR monitor;
};
