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
#include "AMXCommand.h"
#include "IReceiver.h"

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

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	socketServer->setPacketConf(PK_CMP, PK_MSQ);

	if ( FAIL == socketServer->start( AF_INET, NULL, nPort))
	{
		_log("AMX Server Socket Create Fail");
		return FALSE;
	}

	// test
	string strVal = mapAMXCommand[10001];
	_log(strVal.c_str());

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
		nResult = socketServer->socketSend(mnSocketAMX, strCommand.c_str(), strCommand.length());
		_log("[Server AMX] Send Command, length:%d Data:%s", nResult, strCommand.c_str());
	}
	return nResult;
}

int CServerAMX::sendCommand(const int nSocketFD, string strCommand)
{
	int nResult = FALSE;

	if (0 < nSocketFD)
	{
		nResult = socketServer->socketSend(nSocketFD, strCommand.c_str(), strCommand.length());
		_log("[Server AMX] Send Command, length:%d Data:%s", nResult, strCommand.c_str());
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

bool CServerAMX::onReceive(const int nSocketFD, string strCommand)
{
	if (0 < nSocketFD && !strCommand.empty())
	{
		_log("[Server AMX] Receive AMX Command: %s From Socket: %d", strCommand.c_str(), nSocketFD);
		if (0 == strCommand.substr(0, 4).compare("bind"))
		{
			bind(nSocketFD);
		}

		if (0 == strCommand.substr(0, 6).compare("unbind"))
		{
			unbind(nSocketFD);
		}

		if (0 != strCommand.substr(0, 6).compare(CTL_OK) && 0 != strCommand.substr(0, 9).compare(CTL_ERROR))
		{
			sendCommand(nSocketFD, CTL_OK);
		}

		return true;
	}
	else
	{
		sendCommand(nSocketFD, CTL_ERROR);
		_log("[Server AMX] Error Receive AMX Command: %s From Socket: %d", strCommand.c_str(), nSocketFD);
	}
	return false;
}
