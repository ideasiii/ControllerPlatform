/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include "common.h"
#include "LogHandler.h"
#include "CController.h"
#include "CDispatcher.h"
#include "CConfig.h"
#include "event.h"

static CController * controller = 0;

CController::CController() :
		CObject(), dispatcher(CDispatcher::getInstance())
{

}

CController::~CController()
{
	delete dispatcher;
}

CController* CController::getInstance()
{
	if (0 == controller)
	{
		controller = new CController();
	}
	return controller;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{

}

int CController::startDispatcher(const char *szIP, const int nPort, const int nMsqId)
{
	if (dispatcher->start(szIP, nPort))
	{
		dispatcher->idleTimeout(true, 30);
		return TRUE;
	}
	return FALSE;
}

int CController::stop()
{
	dispatcher->stop();
	return FALSE;
}
