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
#include "CServerDevice.h"
#include "CServerMeeting.h"
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

	const CMP_PACKET *cmpPacket = reinterpret_cast<const CMP_PACKET*>(pData);


	_log("[CController] socketFD:%d, Data Length:%d\n", nSocketFD, nDataLen);
	//controlcenter->receiveClientCMP(nSocketFD, nDataLen, pData);
	return 0;
}

void IonMeetingCommand(void *param)
{
	const CMPData *strParam = reinterpret_cast<const CMPData*>(param);
	controller->onMeetingCommand(strParam);
}

void IonDeviceCommand(void *param)
{
	const CMPData *strParam = reinterpret_cast<const CMPData*>(param);
	controller->onDeviceCommand(strParam);
}

void CController::onDeviceCommand(const CMPData * sendBackData)
{
	map<int, CMPData>::iterator itr = deviceMapData.find(sendBackData->nSequence);
	if (itr == deviceMapData.end())
	{
		//not found
		_log("[CController] Cannot find this data which nSequence = %d\n", sendBackData->nSequence);
	}
	else
	{
		//found it!

		CMPData *deviceData = new CMPData(&(itr->second));

		deviceMapData.erase(itr->first);

		serverDevice->sendCommand(deviceData->nFD, sendBackData->nCommand, deviceData->nSequence,
				sendBackData->bodyData);

	}

}

void CController::onMeetingCommand(const CMPData * mCMPData)
{
	int controllerMeetingSeqNum = getSequence();

	deviceMapData[controllerMeetingSeqNum] = *mCMPData;
	serverMeeting->sendCommand(mCMPData->nCommand, controllerMeetingSeqNum, mCMPData->bodyData);
}

/**
 *  Define extern function.
 */
int sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp, CSocket *socket)
{
	if (NULL == socket)
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

	if (isResp)
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
	if (0 != szData)
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
	if (0 >= nRet)
	{
		_log("[Controller] CMP Send Fail socket: %d", nSocket);
	}

	return nRet;
}

CController::CController() :
		CObject(), cmpParser(CCmpHandler::getInstance()), serverMeeting(CServerMeeting::getInstance()), serverDevice(
				CServerDevice::getInstance()), tdEnquireLink(new CThreadHandler), tdExportLog(new CThreadHandler)
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
	case EVENT_COMMAND_SOCKET_TCP_MEETING_RECEIVE:
		_log("[CController] get Controller-Meeting Socket Data from Message Queue");
		serverMeeting->onReceive(nId, pData);
		break;
	case EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE:
		_log("[CController] get Device Socket Data from Message Queue");
		serverDevice->onReceive(nId, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_MEETING:
		serverMeeting->addClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE:
		//serverDevice->addClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_MEETING:
		serverMeeting->deleteClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE:
		//serverDevice->deleteClient(nId);
		break;
	default:
		_log("[Controller] Unknown message command: %d", nCommand);
		break;
	}
}

int CController::startServerMeeting(string strIP, const int nPort, const int nMsqId)
{
	serverMeeting->setCallback(CB_DEVCIE_COMMAND, IonDeviceCommand);
	return serverMeeting->startServer(strIP, nPort, nMsqId);
}

int CController::startServerDevice(string strIP, const int nPort, const int nMsqId)
{
	serverDevice->setCallback(CB_MEETING_COMMAND, IonMeetingCommand);
	return serverDevice->startServer(strIP, nPort, nMsqId);
}

void CController::stopServer()
{
	if (serverMeeting)
	{
		serverMeeting->stopServer();
		delete serverMeeting;
		serverMeeting = 0;
	}

	if (serverDevice)
	{
		serverDevice->stopServer();
		delete serverDevice;
		serverDevice = 0;
	}
}

