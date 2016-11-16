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
#include "utility.h"

static CServerAMX * serverAMX = 0;

CServerAMX::CServerAMX() :
		CSocketServer()
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
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_AMX_RECEIVE);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_AMX);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_AMX);
	}

	/** Set Receive , Packet is BYTE , Message Queue Handle **/
	setPacketConf(PK_BYTE, PK_MSQ);

	if ( FAIL == start( AF_INET, NULL, nPort))
	{
		_log("AMX Server Socket Create Fail");
		return FALSE;
	}
	return TRUE;
}

void CServerAMX::stopServer()
{
	stop();
}

int CServerAMX::sendCommand(string strCommand)
{
	int nResult = FALSE;

	strCommand.append("\n");

	map<int, int>::iterator it;
	for (it = mapClient.begin(); it != mapClient.end(); ++it)
	{
		nResult = socketSend(it->first, strCommand.c_str(), strCommand.length());
		_log("[Server AMX] Send Command, length:%d Data:%s", nResult, strCommand.c_str());
	}
	return nResult;
}

int CServerAMX::sendCommand(const int nSocketFD, string strCommand)
{
	int nResult = FALSE;

	if (0 < nSocketFD)
	{
		nResult = socketSend(nSocketFD, strCommand.c_str(), strCommand.length());
		_log("[Server AMX] Send Command, length:%d Data:%s", nResult, strCommand.c_str());
	}
	return nResult;
}

void CServerAMX::bind(const int nSocketFD)
{
	addClient(nSocketFD);
	_log("[Server AMX] Bind Socket: %d", nSocketFD);
}

void CServerAMX::unbind(const int nSocketFD)
{
	deleteClient(nSocketFD);
	_log("[Server AMX] Unbind Socket: %d", nSocketFD);
}

bool CServerAMX::onReceive(const int nSocketFD, string strCommand)
{
	if (0 < nSocketFD && !strCommand.empty())
	{
		_log("[Server AMX] Receive AMX Command: %s From Socket: %d", strCommand.c_str(), nSocketFD);

		strCommand = trim(strCommand);
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
			// Get Status Response
			if (AMX_STATUS.find(strCommand) != AMX_STATUS.end())
			{
				(*mapCallback[CB_AMX_COMMAND_STATUS])(static_cast<void*>(const_cast<char*>(strCommand.c_str())));

				// Update AMX_STATUS Hashmap
			}
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

void CServerAMX::addClient(const int nSocketFD)
{
	mapClient[nSocketFD] = nSocketFD;
	_log("[Server AMX] Socket Client FD:%d Connected", nSocketFD);
}

void CServerAMX::deleteClient(const int nSocketFD)
{
	mapClient.erase(nSocketFD);
	_log("[Server AMX] Socket Client FD:%d Closed", nSocketFD);
}

void CServerAMX::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}
