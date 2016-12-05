/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include <list>
#include <ctime>

#include "CSocket.h"
#include "CSocketServer.h"
#include "CSocketClient.h"
#include "event.h"
#include "utility.h"
#include "CCmpHandler.h"
#include "CController.h"
#include "CDataHandler.cpp"
#include "CSqliteHandler.h"
#include "CThreadHandler.h"
#include "CServerCenter.h"
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
		CObject(), cmpParser(CCmpHandler::getInstance()), serverCenter(CServerCenter::getInstance()), sqlite(
				CSqliteHandler::getInstance()), tdEnquireLink(new CThreadHandler), tdExportLog(new CThreadHandler)
{

}

CController::~CController()
{
	delete cmpParser;
}

CController* CController::getInstance()
{
	if (0 == controller)
	{
		controller = new CController();
	}
	return controller;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_CENTER_RECEIVE:
		serverCenter->onReceive(nId, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_CENTER:
		serverCenter->addClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_CENTER:
		serverCenter->deleteClient(nId);
		break;
	default:
		_log("[Controller] Unknow message command: %d", nCommand);
		break;
	}
}

int CController::startServerCenter(string strIP, const int nPort, const int nMsqId)
{
	return serverCenter->startServer(strIP, nPort, nMsqId);
}

void CController::stopServer()
{
	if (serverCenter)
	{
		serverCenter->stopServer();
		delete serverCenter;
		serverCenter = 0;
	}
}

int CController::startIdeasSqlite(string strDBPath)
{
	if (strDBPath.empty())
		return FALSE;

	mkdirp(strDBPath);

	if (!sqlite->openIdeasDB(strDBPath.c_str()))
	{
		_log("[Controller] Open Sqlite DB ideas fail");
		return FALSE;
	}

	return TRUE;
}

/**
 * Extern Function Define
 */
int sendCommand(CSocket *socket, const int nSocket, const int nCommandId, const int nStatus, const int nSequence,
		const char * szData)
{
	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	void *pHeader = &packet.cmpHeader;
	char *pIndex = packet.cmpBody.cmpdata;

	memset(&packet, 0, sizeof(CMP_PACKET));

	controller->cmpParser->formatHeader(nCommandId, STATUS_ROK, nSequence, &pHeader);
	if (0 != szData)
	{
		memcpy(pIndex, szData, strlen(szData));
		pIndex += strlen(szData);
		nBody_len += strlen(szData);
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		_log("CMP Body:%s", szData);
	}

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);
	nRet = socket->socketSend(nSocket, &packet, nTotal_len);
	printPacket(nCommandId, nStatus, nSequence, nRet, "[Controller] Send", nSocket);

	string strLog;
	if (0 >= nRet)
	{
		_log("[Controller] CMP Send Fail socket: %d", nSocket);
	}

	return nRet;
}
