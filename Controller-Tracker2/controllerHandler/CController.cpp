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
#include "CClientControllerMongoDB.h"
#include "iCommand.h"
#include "JSONObject.h"
#include "LogHandler.h"

using namespace std;


static bool isInitMongoDB = false;

static CController * controller = 0;

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

/** Enquire link function declare for enquire link thread **/
void *threadEnquireLinkRequest(void *argv);

void IonMongoDBCommand(void *param)
{

	controller->onMongoDBCommand(param);
}

void CController::onMongoDBCommand(void *param)
{
	_log("[CController] call clientMongo deal with it!");
	clientMongo->sendCommand(param);
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
		CObject(), cmpParser(CCmpHandler::getInstance()), serverDevice(CServerDevice::getInstance()), tdEnquireLink(
				new CThreadHandler), clientMongo(CClientControllerMongoDB::getInstance()),clientMongoDBPort(
				-1), clientMongoDBMsqId(-1)

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
	int status = 0;
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE:
		_log("[CController] get Device Socket Data from Message Queue");
		serverDevice->onReceive(nId, pData);
		break;

	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE:
		serverDevice->addClient(nId);
		break;

	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE:
		serverDevice->deleteClient(nId);
		break;

	case EVENT_COMMAND_SOCKET_TCP_MONGODB_RECEIVE:
		clientMongo->onReceive(nId, pData);
		break;

	case EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MONGODB:
		_log("[Controller]****MongoDB disconnect!*****");

		break;

	case EVENT_COMMAND_RECONNECT_CONTROLLER_MONGODB:

		_log("[CController]***Start to ReConnect Controller-MongoDB***\n");

		clientMongo = CClientControllerMongoDB::getInstance();
		status = reStartClientMongoDB();

		_log("[CController] ReConnect Controller-MongoDB, status: %d\n", status);
		break;
	default:
		_log("[Controller] Unknown Message Command: %d", nCommand);
		break;
	}
}

int CController::startServerDevice(string strIP, const int nPort, const int nMsqId)
{
	serverDevice->setCallback(CB_CONTROLLER_MONGODB_COMMAND, IonMongoDBCommand);
	return serverDevice->startServer(strIP, nPort, nMsqId);
}

int CController::reStartClientMongoDB()
{
	if (clientMongoDBIP.empty() || clientMongoDBPort == -1 || clientMongoDBMsqId == -1)
	{
		return -1;
	}
	else
	{
		return startClientMongoDB(clientMongoDBIP, clientMongoDBPort, clientMongoDBMsqId);
	}
}

int CController::startClientMongoDB(string strIP, const int nPort, const int nMsqId)
{
	if (isInitMongoDB == false)
	{
		clientMongoDBIP = strIP;
		clientMongoDBPort = nPort;
		clientMongoDBMsqId = nMsqId;
		isInitMongoDB = true;
		tdEnquireLink->createThread(threadEnquireLinkRequest, this);
	}
	_log("[Controller] start to connect IP: %s, port: %d \n", strIP.c_str(), nPort);

	if (clientMongo->startClient(strIP, nPort, nMsqId) == TRUE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void CController::stopServer()
{

	if (clientMongo)
	{
		clientMongo->stopClient();
		delete clientMongo;
		clientMongo = 0;
	}
	if (serverDevice)
	{
		serverDevice->stopServer();
		delete serverDevice;
		serverDevice = 0;
	}
}

void CController::runEnquireLinkRequest()
{
	while (1)
	{
		tdEnquireLink->threadSleep(600);
		_log("[CController] Enquire Link Start\n");
		if (!clientMongo)
		{
			clientMongo = CClientControllerMongoDB::getInstance();
		}
		if (clientMongo->isValidSocketFD())
		{
			int nRet = cmpEnquireLinkRequest(clientMongo->getSocketfd());

			if (nRet > 0)
			{
				//Enquire Link Success
			}
			else
			{
				//Enquire Link Failed
				_log("[CController] Send Enquire Link Failed result = %d\n", nRet);
				int status = controller->sendMessage(EVENT_FILTER_CONTROLLER,
				EVENT_COMMAND_RECONNECT_CONTROLLER_MONGODB, 0, 0, NULL);
			}
		}
		else
		{
			_log("[CController] ERROR to find Controller-MongoDB Socket ID!\n");

			controller->sendMessage(EVENT_FILTER_CONTROLLER, EVENT_COMMAND_RECONNECT_CONTROLLER_MONGODB, 0, 0,
			NULL);

		}

	}

}

int CController::sendCommand(int commandID, int seqNum)
{
	int nRet = 0;
	if (clientMongo->isValidSocketFD())
	{
		nRet = sendPacket(dynamic_cast<CSocket*>(clientMongo), clientMongo->getSocketfd(), commandID, STATUS_ROK,
				seqNum, 0);
	}

	return nRet;
}

int CController::cmpEnquireLinkRequest(const int nSocketFD)
{
	return sendCommand(enquire_link_request, getSequence());
}

/************************************* thread function **************************************/
void *threadEnquireLinkRequest(void *argv)
{
	CController* ss = reinterpret_cast<CController*>(argv);
	ss->runEnquireLinkRequest();
	return NULL;
}

