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
#include "CThreadHandler.h"
#include "CServerAMX.h"
#include "CServerDevice.h"
#include "AMXCommand.h"
#include "iCommand.h"
#include "JSONObject.h"
#include "packet.h"

using namespace std;

static CController * controller = 0;

/** Callback Function AMX Request from Mobile **/
void IonAMXCommandControl(void* param)
{
	string strParam = reinterpret_cast<const char*>(param);
	controller->onAMXCommand(strParam);
}

void IonAMXCommandStatus(void *param)
{
	string strParam = reinterpret_cast<const char*>(param);
	controller->onAMXCommand(strParam);
}

void CController::onAMXCommand(string strCommand)
{
	serverAMX->sendCommand(strCommand);
}

/** Callback Function AMX Response from AMX **/
void IonAMXResponseStatus(void *param)
{
	string strParam = reinterpret_cast<const char*>(param);
	controller->onAMXResponseStatus(strParam);
}

void CController::onAMXResponseStatus(string strStatus)
{
	serverDevice->broadcastAMXStatus(strStatus);
}

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

/**
 *  Define extern function.
 */
int sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp, CSocket *socket)
{
	if(NULL == socket)
	{
		_log("[Controller] Send Command Fail, Socket invalid");
		return -1;
	}
	int nRet = -1;
	int nCommandSend;
	CMP_HEADER cmpHeader;
	void *pHeader = &cmpHeader;

	memset(&cmpHeader, 0, sizeof(CMP_HEADER));
	nCommandSend = nCommand;

	if(isResp)
	{
		nCommandSend = generic_nack | nCommand;
	}

	controller->cmpParser->formatHeader(nCommandSend, nStatus, nSequence, &pHeader);
	nRet = socket->socketSend(nSocket, &cmpHeader, sizeof(CMP_HEADER));
	printPacket(nCommandSend, nStatus, nSequence, nRet, "[Controller] Send", nSocket);
	return nRet;
}

int cmpSend(CSocket *socket, const int nSocket, const int nCommandId, const int nSequence, const char * szData)
{
	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	void *pHeader = &packet.cmpHeader;
	char *pIndex = packet.cmpBody.cmpdata;

	memset(&packet, 0, sizeof(CMP_PACKET));

	controller->cmpParser->formatHeader(nCommandId, STATUS_ROK, nSequence, &pHeader);
	if(0 != szData)
	{
		memcpy(pIndex, szData, strlen(szData));
		pIndex += strlen(szData);
		nBody_len += strlen(szData);
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
	}

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);
	nRet = socket->socketSend(nSocket, &packet, nTotal_len);
	printPacket(nCommandId, STATUS_ROK, nSequence, nRet, "[Controller] Send", nSocket);

	string strLog;
	if(0 >= nRet)
	{
		_log("[Controller] CMP Send Fail socket: %d", nSocket);
	}

	return nRet;
}

CController::CController() :
		CObject(), cmpParser(CCmpHandler::getInstance()), serverAMX(CServerAMX::getInstance()), serverDevice(
				CServerDevice::getInstance()), tdEnquireLink(new CThreadHandler), tdExportLog(new CThreadHandler)
{

}

CController::~CController()
{
	delete cmpParser;
}

CController* CController::getInstance()
{
	if(0 == controller)
	{
		controller = new CController();
	}
	return controller;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch(nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_AMX_RECEIVE:
		serverAMX->onReceive(nId, static_cast<char*>(const_cast<void*>(pData)));
		break;
	case EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE:
		serverDevice->onReceive(nId, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_AMX:
		serverAMX->addClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE:
		serverDevice->addClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_AMX:
		serverAMX->deleteClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE:
		serverDevice->deleteClient(nId);
		break;
	default:
		_log("[Controller] Unknow message command: %d", nCommand);
		break;
	}
}

int CController::startServerAMX(string strIP, const int nPort, const int nMsqId)
{
	serverAMX->setCallback(CB_AMX_COMMAND_STATUS, IonAMXResponseStatus);
	return serverAMX->startServer(strIP, nPort, nMsqId);
}

int CController::startServerDevice(string strIP, const int nPort, const int nMsqId)
{
	serverDevice->setCallback(CB_AMX_COMMAND_CONTROL, IonAMXCommandControl);
	serverDevice->setCallback(CB_AMX_COMMAND_STATUS, IonAMXCommandStatus);
	return serverDevice->startServer(strIP, nPort, nMsqId);
}

void CController::stopServer()
{
	if(serverAMX)
	{
		serverAMX->stopServer();
		delete serverAMX;
		serverAMX = 0;
	}

	if(serverDevice)
	{
		serverDevice->stopServer();
		delete serverDevice;
		serverDevice = 0;
	}
}

void CController::setAMXBusyTimer(int nSec)
{
	serverDevice->setAmxBusyTimeout(nSec);
}
