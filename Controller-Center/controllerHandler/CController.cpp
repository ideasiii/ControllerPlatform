/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include <string>
#include "event.h"
#include "CController.h"
#include "CServerCenter.h"
#include "LogHandler.h"
#include "CSqliteHandler.h"
#include "utility.h"
#include "packet.h"

using namespace std;

static CController * controller = 0;

/**
 * Define Socket Client ReceiveFunction
 */
int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
	//controlcenter->receiveCMP(nSocketFD, nDataLen, pData);
	return 0;
}

/**
 *  Define Socket Server Receive Function
 */
int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
{
	//controlcenter->receiveClientCMP(nSocketFD, nDataLen, pData);
	return 0;
}

CController::CController() :
		CObject(), serverCenter(CServerCenter::getInstance())
{

}

CController::~CController()
{

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
	case EVENT_COMMAND_SOCKET_TCP_CENTER_RECEIVE:
		serverCenter->onReceive(nId, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_CENTER:
		serverCenter->addClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_CENTER:
		serverCenter->deleteClient(nId);
		break;
	default:
		_log("[Controller] Unknow message command: %d", nCommand);
		break;
	}
}

int CController::startServerCenter(const char* szIP, const int nPort, const int nMsqId)
{
	return serverCenter->startServer(szIP, nPort, nMsqId);
}

void CController::stopServer()
{
	if(serverCenter)
	{
		serverCenter->stopServer();
		delete serverCenter;
		serverCenter = 0;
	}
}
