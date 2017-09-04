/*
 * CController.cpp
 *
 *  Created on: 2017年8月18日
 *      Author: Jugo
 */

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

	manager = new CManager(this);
	if(config.loadConfig(reinterpret_cast<const char*>(szConfPath)))
	{
		strValue = config.getValue("PROCESS", "thread_max");
		if(!strValue.empty())
		{
			convertFromString(nValue, strValue);
			manager->setThreadMax(nValue);
		}
	}

	setTimer(ID_TIMER_CHECK_PROCESS, 5, 5);
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

	}
}

void CController::onTimer(int nId)
{
	switch(nId)
	{
	case ID_TIMER_CHECK_PROCESS:
		manager->checkProcess();
		break;
	}
}

