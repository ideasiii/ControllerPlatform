/*
 * CServerDevice.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: root
 */

#include "CServerDevice.h"
#include "CSocketServer.h"
#include "IReceiver.h"
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

int CServerDevice::startServer(const int nPort, const int nMsqId)
{
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if (0 < nMsqId)
	{
		socketServer->setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE);
		socketServer->setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE);
		socketServer->setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	socketServer->setPacketConf(PK_CMP, PK_MSQ);

	if ( FAIL == socketServer->start( AF_INET, NULL, nPort))
	{
		_log("Device Server Socket Create Fail");
		return FALSE;
	}

	return TRUE;
}

void CServerDevice::stopServer()
{
	if (socketServer)
	{
		socketServer->stop();
		delete socketServer;
		socketServer = 0;
	}
}

int CServerDevice::sendCommand(const int nSocketFD, string strCommand)
{
	int nResult = FALSE;

	if (0 < nSocketFD)
	{
		nResult = socketServer->socketSend(nSocketFD, strCommand.c_str(), strCommand.length());
		_log("[Server Device] Send Command, length:%d Data:%s", nResult, strCommand.c_str());
	}
	return nResult;
}

