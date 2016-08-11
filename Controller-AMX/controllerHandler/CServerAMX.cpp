/*
 * CServerAMX.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#include "CServerAMX.h"
#include "CSocketServer.h"
#include "event.h"
#include "packet.h"
#include "common.h"

static CServerAMX * serverAMX = 0;

CServerAMX::CServerAMX() :
		socketServer(new CSocketServer), mnSocketAMX(-1)
{

}

CServerAMX::~CServerAMX()
{
	stopServer();
}

CServerAMX * CServerAMX::getInstance()
{
	if (0 == serverAMX)
	{
		serverAMX = new CServerAMX();
	}
	return serverAMX;
}

int CServerAMX::startServer(const int nPort, const int nMsqId)
{
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if (0 < nMsqId)
	{
		socketServer->setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_AMX_RECEIVE);
		socketServer->setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_AMX);
		socketServer->setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_AMX);
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

int CServerAMX::sendCommand(string strCommand)
{
	int nResult = FALSE;

	if (0 < mnSocketAMX)
	{
		int nRet = socketServer->socketSend(mnSocketAMX, strCommand.c_str(), strCommand.length());
		_log("[Server AMX] Send Command, length:%d Data:%s", nRet, strCommand.c_str());
	}
	return nResult;
}

void CServerAMX::bind(const int nSocketFD)
{
	mnSocketAMX = nSocketFD;
	_log("[Server AMX] Bind Socket: %d", mnSocketAMX);
}

void CServerAMX::unbind(const int nSocketFD)
{

	if (nSocketFD == mnSocketAMX)
	{
		_log("[Server AMX] Unbind Socket: %d", mnSocketAMX);
		mnSocketAMX = -1;
	}
}
