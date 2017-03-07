/*
 * CDispatcher.cpp
 *
 *  Created on: 2017年3月7日
 *      Author: root
 */

#include "CDispatcher.h"
#include "common.h"
#include "LogHandler.h"
#include "event.h"
#include "IReceiver.h"

static CDispatcher * dispatcher = 0;

/**
 * Define Socket Client ReceiveFunction
 */
int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

/**
 *  Define Socket Server Receive Function
 */
int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

CDispatcher* CDispatcher::getInstance()
{
	if(0 == dispatcher)
	{
		dispatcher = new CDispatcher();
	}
	return dispatcher;
}

CDispatcher::CDispatcher()
{

}

CDispatcher::~CDispatcher()
{

}

int CDispatcher::startServer(const char *szIP, const int nPort, const int nMsqId)
{
	if(0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if(0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_DISPATCHER_RECEIVER);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DISPATCHER);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DISPATCHER);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	setPacketConf(PK_CMP, PK_MSQ);

	if(FAIL == start(AF_INET, szIP, nPort))
	{
		_log("[CDispatcher] Socket Create Fail");
		return FALSE;
	}

	return TRUE;
}

void CDispatcher::stopServer()
{
	this->stop();
}

void CDispatcher::onReceiveMessage(unsigned long int nId, int nDataLen, const void* pData)
{

}

void CDispatcher::setClient(unsigned long int nId, bool bAdd)
{
	if(bAdd)
	{
		listClient[nId] = 1;
	}
	else
	{
		listClient.erase(nId);
	}
}

void CDispatcher::checkClient()
{
	_log("[CDispatcher] checkClient");
}

