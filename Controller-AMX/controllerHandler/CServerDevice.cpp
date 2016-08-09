/*
 * CServerDevice.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: root
 */

#include "CServerDevice.h"
#include "CSocketServer.h"
#include "event.h"
#include "packet.h"

static CServerDevice * serverDevice = 0;

CServerDevice::CServerDevice() :
		socketServer(new CSocketServer)
{

}

CServerDevice::~CServerDevice()
{
	if (0 != socketServer)
	{
		socketServer->stop();
		delete socketServer;
	}
}

CServerDevice * CServerDevice::getInstance()
{
	if (0 == serverDevice)
	{
		serverDevice = new CServerDevice();
	}
	return serverDevice;
}

int CServerAMX::startServer(const int nPort, const int nMsqId)
{
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if (0 < nMsqId)
	{
		socketServer->setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE);
		socketServer->setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT);
		socketServer->setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT);
	}

	if ( FAIL == socketServer->start( AF_INET, NULL, nPort))
	{
		_log("AMX Server Socket Create Fail");
		return FALSE;
	}

	return TRUE;
}

void CServerAMX::stopServer()
{
	if (socketServer)
	{
		socketServer->stop();
		delete socketServer;
		socketServer = 0;
	}
}

