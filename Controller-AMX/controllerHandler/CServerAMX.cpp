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

static CServerAMX * serverAMX = 0;

CServerAMX::CServerAMX() :
		socketServer(new CSocketServer)
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
		socketServer->setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT);
		socketServer->setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT);
	}

	if ( FAIL == socketServer->start( AF_INET, NULL, nPort))
	{
		_log("AMX Server Socket Create Fail");
		return FALSE;
	}

	//tdEnquireLink->createThread(threadEnquireLinkRequest, this);

	return TRUE;
}

void CServerAMX::stopServer()
{
//	if (tdEnquireLink)
//	{
//		tdEnquireLink->threadExit();
//		delete tdEnquireLink;
//		_DBG("[Controller] Stop Enquire Link Thread");
//	}

	if (socketServer)
	{
		socketServer->stop();
		delete socketServer;
		socketServer = 0;
	}
}
