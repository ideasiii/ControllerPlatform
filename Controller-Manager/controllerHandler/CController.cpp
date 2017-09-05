/*
 * CController.cpp
 *
 *  Created on: 2017年8月18日
 *      Author: Jugo
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "CController.h"
#include "common.h"
#include "CManager.h"
#include "event.h"
#include "CConfig.h"
#include "utility.h"

#define ID_TIMER_CHECK_PROCESS		7777

using namespace std;

CController::CController() :
		mnMsqKey(0), manager(0)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_MANAGER;
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	CConfig config;
	string strValue;
	int nValue;

	stTimer.nSecStart = 5;
	stTimer.nInterSec = 5;

	manager = new CManager(this);
	if(config.loadConfig(reinterpret_cast<const char*>(szConfPath)))
	{
		strValue = config.getValue("TIMER", "SecStart");
		if(!strValue.empty())
			convertFromString(stTimer.nSecStart, strValue);

		strValue = config.getValue("TIMER", "InterSec");
		if(!strValue.empty())
			convertFromString(stTimer.nInterSec, strValue);

		strValue = config.getValue("PROCESS", "thread_max");
		if(!strValue.empty())
		{
			convertFromString(nValue, strValue);
			manager->setThreadMax(nValue);
		}

		strValue = config.getValue("PROCESS", "total");
		if(!strValue.empty())
		{
			convertFromString(nValue, strValue);
			for(int i = 1; i <= nValue; ++i)
			{
				strValue = config.getValue("PROCESS", format("%d", i));
				if(!strValue.empty())
				{
					manager->addProcess(strValue.c_str());
				}
			}
		}
	}

	if(manager->getProcessCount())
		setTimer(ID_TIMER_CHECK_PROCESS, stTimer.nSecStart, stTimer.nInterSec);
	else
		_log("!! ============ WARNING !! No Process to Monitor =============== !!");
	return TRUE;
}

int CController::onFinish(void* nMsqKey)
{
	killTimer(ID_TIMER_CHECK_PROCESS);
	delete manager;
	return FALSE;
}

void CController::onHandleMessage(Message &message)
{
	switch(message.what)
	{
	case PROC_NOT_RUN:
		_log("[CController] onHandleMessage %s not Run, will launch it", message.strData.c_str());
		launchProcess(message.strData.c_str());
		break;
	case PROC_THREAD_OVER:
		_log("[CController] onHandleMessage %s Thread Over then kill it", message.strData.c_str());
		killProcess(message.strData.c_str());
		break;
	}
	setTimer(ID_TIMER_CHECK_PROCESS, stTimer.nSecStart, stTimer.nInterSec);
}

void CController::onTimer(int nId)
{
	switch(nId)
	{
	case ID_TIMER_CHECK_PROCESS:
		if(!manager->checkProcess())
			killTimer(ID_TIMER_CHECK_PROCESS);
		break;
	}
}

void CController::launchProcess(const char *szProcName)
{
	string strShell;

	if(szProcName)
	{
		strShell = format("./%s.sh", szProcName);
		system(strShell.c_str());
	}
}

void CController::killProcess(const char *szProcName)
{
	string strShell;

	if(szProcName)
	{
		strShell = format("killall %s", szProcName);
		system(strShell.c_str());
	}
}

