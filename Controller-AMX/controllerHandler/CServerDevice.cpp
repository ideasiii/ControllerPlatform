/*
 * CServerDevice.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: root
 */

#include "CServerDevice.h"
#include "IReceiver.h"
#include "event.h"
#include "packet.h"
#include "common.h"

static CServerDevice * serverDevice = 0;

CServerDevice::CServerDevice() :
		CSocketServer()
{

}

CServerDevice::~CServerDevice()
{
	stop();
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
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	setPacketConf(PK_CMP, PK_MSQ);

	if (FAIL == start( AF_INET, NULL, nPort))
	{
		_log("Device Server Socket Create Fail");
		return FALSE;
	}

	return TRUE;
}

void CServerDevice::stopServer()
{
	stop();
}

