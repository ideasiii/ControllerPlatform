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

#define TIMER_CHECK_DISPATCH_CLIENT_ALIVE 777

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
	if(0 == controller)
	{
		controller = new CController();
	}
	return controller;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch(nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_DISPATCHER_RECEIVER:
		dispatcher->onReceiveMessage(nId, nDataLen, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DISPATCHER:
		dispatcher->setClient(nId, true);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DISPATCHER:
		dispatcher->setClient(nId, false);
		break;
	case EVENT_COMMAND_TIMER:
		switch(nId)
		{
		case TIMER_CHECK_DISPATCH_CLIENT_ALIVE:
			dispatcher->checkClient();
			break;
		}
		break;
	}
}

void CController::onTimer(int nId)
{
	this->sendMessage(EVENT_FILTER_CONTROLLER, EVENT_COMMAND_TIMER, TIMER_CHECK_DISPATCH_CLIENT_ALIVE, 0, 0);
}

int CController::startDispatcher(const char *szIP, const int nPort, const int nMsqId)
{
	if(dispatcher->startServer(szIP, nPort, nMsqId))
	{
		setTimer(TIMER_CHECK_DISPATCH_CLIENT_ALIVE, 5, 10);
		return TRUE;
	}
	return FALSE;
}

int CController::stop()
{
	dispatcher->stopServer();
	killTimer(TIMER_CHECK_DISPATCH_CLIENT_ALIVE);
	return FALSE;
}
