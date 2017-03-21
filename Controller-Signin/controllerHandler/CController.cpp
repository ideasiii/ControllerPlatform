/*
 * CController.cpp
 *
 *  Created on: 2017年03月14日
 *      Author: Jugo
 */

#include "common.h"
#include "LogHandler.h"
#include "CController.h"
#include "CSignin.h"
#include "CConfig.h"
#include "event.h"
#include "CCmpSignin.h"

static CController * controller = 0;

CController::CController() :
		CObject(), signin(CSignin::getInstance()), cmpSignin(new CCmpSignin())
{

}

CController::~CController()
{
	delete cmpSignin;
	delete signin;
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
	case EVENT_COMMAND_SOCKET_TCP_SIGNIN_RECEIVER:
		signin->onReceiveMessage(nId, nDataLen, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DISPATCHER:
		signin->setClient(nId, true);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DISPATCHER:
		signin->setClient(nId, false);
		break;
	}
}

void CController::onTimer(int nId)
{
	_DBG("[CController] onTimer Id: %d", nId);
	//this->sendMessage(EVENT_FILTER_CONTROLLER, EVENT_COMMAND_TIMER, TIMER_CHECK_DISPATCH_CLIENT_ALIVE, 0, 0);
}

int CController::startSignin(const char *szIP, const int nPort, const int nMsqId)
{
	if(cmpSignin->start(szIP, nPort))
	{
		return TRUE;
	}
	return FALSE;
}

int CController::stop()
{
	cmpSignin->stop();
	signin->stopServer();
	return FALSE;
}
