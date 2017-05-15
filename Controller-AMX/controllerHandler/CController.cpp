/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include "CController.h"
#include "CServerAMX.h"
#include "common.h"
#include "config.h"
#include "CConfig.h"
#include "CServerCMP.h"

using namespace std;

//void IonAMXCommandControl(void* param)
//{
//	string strParam = reinterpret_cast<const char*>(param);
//	controller->onAMXCommand(strParam);
//}
//
//void IonAMXCommandStatus(void *param)
//{
//	string strParam = reinterpret_cast<const char*>(param);
//	controller->onAMXCommand(strParam);
//}
//
//void CController::onAMXCommand(string strCommand)
//{
//	serverAMX->sendCommand(strCommand);
//}
//
///** Callback Function AMX Response from AMX **/
//void IonAMXResponseStatus(void *param)
//{
//	string strParam = reinterpret_cast<const char*>(param);
//	controller->onAMXResponseStatus(strParam);
//}
//
//void CController::onAMXResponseStatus(string strStatus)
//{
//	serverDevice->broadcastAMXStatus(strStatus);
//}
//
///**
// * Define Socket Client ReceiveFunction
// */
//int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
//{
//	//controlcenter->receiveCMP(nSocketFD, nDataLen, pData);
//	return 0;
//}
//
///**
// *  Define Socket Server Receive Function
// */
//int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
//{
//	//controlcenter->receiveClientCMP(nSocketFD, nDataLen, pData);
//	return 0;
//}

/**
 *  Define extern function.
 */
//int sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp, CSocket *socket)
//{
//	if(NULL == socket)
//	{
//		_log("[Controller] Send Command Fail, Socket invalid");
//		return -1;
//	}
//	int nRet = -1;
//	int nCommandSend;
//	CMP_HEADER cmpHeader;
//	void *pHeader = &cmpHeader;
//
//	memset(&cmpHeader, 0, sizeof(CMP_HEADER));
//	nCommandSend = nCommand;
//
//	if(isResp)
//	{
//		nCommandSend = generic_nack | nCommand;
//	}
//
//	controller->cmpParser->formatHeader(nCommandSend, nStatus, nSequence, &pHeader);
//	nRet = socket->socketSend(nSocket, &cmpHeader, sizeof(CMP_HEADER));
//	printPacket(nCommandSend, nStatus, nSequence, nRet, "[Controller] Send", nSocket);
//	return nRet;
//}
//
//int cmpSend(CSocket *socket, const int nSocket, const int nCommandId, const int nSequence, const char * szData)
//{
//	int nRet = -1;
//	int nBody_len = 0;
//	int nTotal_len = 0;
//
//	CMP_PACKET packet;
//	void *pHeader = &packet.cmpHeader;
//	char *pIndex = packet.cmpBody.cmpdata;
//
//	memset(&packet, 0, sizeof(CMP_PACKET));
//
//	controller->cmpParser->formatHeader(nCommandId, STATUS_ROK, nSequence, &pHeader);
//	if(0 != szData)
//	{
//		memcpy(pIndex, szData, strlen(szData));
//		pIndex += strlen(szData);
//		nBody_len += strlen(szData);
//		memcpy(pIndex, "\0", 1);
//		pIndex += 1;
//		nBody_len += 1;
//	}
//
//	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
//	packet.cmpHeader.command_length = htonl(nTotal_len);
//	nRet = socket->socketSend(nSocket, &packet, nTotal_len);
//	printPacket(nCommandId, STATUS_ROK, nSequence, nRet, "[Controller] Send", nSocket);
//
//	string strLog;
//	if(0 >= nRet)
//	{
//		_log("[Controller] CMP Send Fail socket: %d", nSocket);
//	}
//
//	return nRet;
//}
CController::CController() :
		mnMsqKey(-1), serverAMX(0), serverDevice(0)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	serverAMX = new CServerAMX(this);
	serverDevice = new CServerDevice(this);
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_AMX;
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nSecond;
	int nRet;
	int nPort;
	string strPort;
	CConfig *config;
	string strConfPath;
	string strTimer;

	nRet = FALSE;
	strConfPath = reinterpret_cast<const char*>(szConfPath);
	if(strConfPath.empty())
		return nRet;

	_log("[CController] onInitial Config File: %s", strConfPath.c_str());

	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		strPort = config->getValue("SERVER AMX", "port");
		if(!strPort.empty())
		{
			convertFromString(nPort, strPort);
			if(startServerAMX(nPort, mnMsqKey))
			{
				strPort = config->getValue("SERVER DEVICE", "port");
				if(!strPort.empty())
				{
					convertFromString(nPort, strPort);
					if(startServerDevice(nPort, mnMsqKey))
					{
						strTimer = config->getValue("TIMER", "amx_busy");
						if(!strTimer.empty())
						{
							convertFromString(nSecond, strTimer);
							serverDevice->setAmxBusyTimeout(nSecond);
						}
						nRet = TRUE;
					}
				}
			}
		}

	}
	delete config;
	return nRet;
}

int CController::onFinish(void* nMsqKey)
{
	if(serverAMX)
	{
		serverAMX->stop();
		delete serverAMX;
		serverAMX = 0;
	}

	if(serverDevice)
	{
		serverDevice->stop();
		delete serverDevice;
		serverDevice = 0;
	}

	return TRUE;
}

void CController::onHandleMessage(Message &message)
{
	switch(message.what)
	{
	case RESPONSE_AMX_STATUS:
		break;
	}
}

//void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
//{
//	switch(nCommand)
//	{
//	case EVENT_COMMAND_SOCKET_TCP_AMX_RECEIVE:
//		serverAMX->onReceive(nId, static_cast<char*>(const_cast<void*>(pData)));
//		break;
//	case EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE:
//		serverDevice->onReceive(nId, pData);
//		break;
//	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_AMX:
//		serverAMX->addClient(nId);
//		break;
//	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE:
//		serverDevice->addClient(nId);
//		break;
//	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_AMX:
//		serverAMX->deleteClient(nId);
//		break;
//	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE:
//		serverDevice->deleteClient(nId);
//		break;
//	default:
//		_log("[Controller] Unknow message command: %d", nCommand);
//		break;
//	}
//}

int CController::startServerAMX(const int nPort, const int nMsqId)
{
	return serverAMX->start(0, nPort, nMsqId);
}

int CController::startServerDevice(const int nPort, const int nMsqId)
{
	return serverDevice->start(0, nPort, nMsqId);
}
