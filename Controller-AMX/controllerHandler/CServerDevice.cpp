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
#include "CCmpHandler.h"

static CServerDevice * serverDevice = 0;

CServerDevice::CServerDevice() :
		CSocketServer(), cmpParser(CCmpHandler::getInstance())
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

void CServerDevice::onReceive(const int nSocketFD, const void *pData, CBFun cbfun)
{
	int nRet = -1;
	int nPacketLen = 0;
	CMP_HEADER cmpHeader;
	char *pPacket;

	pPacket = (char*) const_cast<void*>(pData);
	memset(&cmpHeader, 0, sizeof(CMP_HEADER));

	cmpHeader.command_id = cmpParser->getCommand(pPacket);
	cmpHeader.command_length = cmpParser->getLength(pPacket);
	cmpHeader.command_status = cmpParser->getStatus(pPacket);
	cmpHeader.sequence_number = cmpParser->getSequence(pPacket);

	printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
			"[Controller Recv]", nSocketFD);

	if (cmpParser->isAckPacket(cmpHeader.command_id))
	{
		return;
	}

	(*cbfun)(static_cast<void*>(const_cast<char*>("dddddxxxxxx")));
}

