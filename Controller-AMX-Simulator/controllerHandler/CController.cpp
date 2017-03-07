/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include <ctime>

#include "CSocket.h"
#include "CSocketServer.h"
#include "CSocketClient.h"
#include "event.h"
#include "utility.h"
#include "CCmpHandler.h"
#include "CController.h"
#include "CDataHandler.cpp"
#include "CServerDevice.h"
#include "AMXCommand.h"
#include "JSONObject.h"
#include "ICallback.h"

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
		CObject(), serverDevice(CServerDevice::getInstance())
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
	case EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE:
		serverDevice->onReceive(nId, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE:
		serverDevice->addClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE:
		serverDevice->deleteClient(nId);
		break;
	default:
		_log("[Controller] Unknow message command: %d", nCommand);
		break;
	}
}

int CController::startServerDevice(const char *szIP, const int nPort, const int nMsqId)
{
	return serverDevice->startServer(szIP, nPort, nMsqId);
}

void CController::stopServer()
{
	if(serverDevice)
	{
		serverDevice->stopServer();
		delete serverDevice;
		serverDevice = 0;
	}
}

void CController::setAMXBusyTimer(int nSec)
{
	serverDevice->setAmxBusyTimeout(nSec);
}
