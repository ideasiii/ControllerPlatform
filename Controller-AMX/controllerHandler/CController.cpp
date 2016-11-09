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
#include "CServerAMX.h"
#include "CServerDevice.h"
#include "AMXCommand.h"
#include "JSONObject.h"
#include "ICallback.h"

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

/** Enquire link function declare for enquire link thread **/
void *threadEnquireLinkRequest(void *argv);

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
	if (NULL == socket)
	{
		_log("Send Command Fail, Socket invalid");
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
	printPacket(nCommandSend, nStatus, nSequence, nRet, "[Controller Send]", nSocket);
	return nRet;
}

CController::CController() :
		CObject(), cmpParser(CCmpHandler::getInstance()), serverAMX(CServerAMX::getInstance()), serverDevice(
				CServerDevice::getInstance()), sqlite(CSqliteHandler::getInstance()), tdEnquireLink(new CThreadHandler), tdExportLog(
				new CThreadHandler)
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

int CController::startSqlite(const int nDBId, const std::string strDB)
{
	int nResult = FALSE;
	switch (nDBId)
	{
	case DB_CONTROLLER:
		nResult = sqlite->openControllerDB(strDB.c_str());
		break;
	case DB_USER:
		nResult = sqlite->openUserDB(strDB.c_str());
		break;
	case DB_IDEAS:
		nResult = sqlite->openIdeasDB(strDB.c_str());
		break;
	case DB_MDM:
		nResult = sqlite->openMdmDB(strDB.c_str());
		break;
	}

	return nResult;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_AMX_RECEIVE:
		serverAMX->onReceive(nId, static_cast<char*>(const_cast<void*>(pData)));
		break;
	case EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE:
		serverDevice->onReceive(nId, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_AMX:
		serverAMX->addAMXClient(nId);
		_log("[Controller] AMX Socket Client FD:%d Connected", (int) nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE:
		serverDevice->addClient(nId);
		_log("[Controller] Device Socket Client FD:%d Connected", (int) nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_AMX:
		serverAMX->deleteAMXClient(nId);
		_log("[Controller] AMX Socket Client FD:%d Close", (int) nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE:
		serverDevice->deleteClient(nId);
		_log("[Controller] Device Socket Client FD:%d Close", (int) nId);
		break;
	default:
		_log("[Controller] Unknow message command: %d", nCommand);
		break;
	}
}

int CController::startServerAMX(const int nPort, const int nMsqId)
{
	serverAMX->setCallback(CB_AMX_COMMAND_STATUS, IonAMXResponseStatus);
	return serverAMX->startServer(nPort, nMsqId);
}

int CController::startServerDevice(const int nPort, const int nMsqId)
{
	serverDevice->setCallback(CB_AMX_COMMAND_CONTROL, IonAMXCommandControl);
	serverDevice->setCallback(CB_AMX_COMMAND_STATUS, IonAMXCommandStatus);
	return serverDevice->startServer(nPort, nMsqId);
}

void CController::stopServer()
{
	if (serverAMX)
	{
		serverAMX->stopServer();
		delete serverAMX;
		serverAMX = 0;
	}

	if (serverDevice)
	{
		serverDevice->stopServer();
		delete serverDevice;
		serverDevice = 0;
	}
}

int CController::getControllerSocketFD(std::string strControllerID)
{
	int nRet = FAIL;
	string strSQL = "SELECT socket_fd FROM controller WHERE status = 1 and id = '" + strControllerID + "';";
	list<int> listValue;

	if (0 < sqlite->getControllerColumeValueInt(strSQL.c_str(), listValue, 0))
	{
		list<int>::iterator i = listValue.begin();
		nRet = *i;
	}
	listValue.clear();
	return nRet;
}

int CController::cmpResponse(const int nSocket, const int nCommandId, const int nSequence, const char * szData)
{
	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	void *pHeader = &packet.cmpHeader;
	char *pIndex = packet.cmpBody.cmpdata;

	memset(&packet, 0, sizeof(CMP_PACKET));

	cmpParser->formatHeader(nCommandId, STATUS_ROK, nSequence, &pHeader);
	memcpy(pIndex, szData, strlen(szData));
	pIndex += strlen(szData);
	nBody_len += strlen(szData);
	memcpy(pIndex, "\0", 1);
	pIndex += 1;
	nBody_len += 1;

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);

//	nRet = cmpServer->socketSend(nSocket, &packet, nTotal_len);
	printPacket(nCommandId, STATUS_ROK, nSequence, nRet, "[Controller] cmpResponse", nSocket);

	string strLog;
	if (0 >= nRet)
	{
		_log("[Controller] cmpResponse Fail socket: %d", nSocket);
	}

	return nRet;
}

void CController::runEnquireLinkRequest()
{
	int nSocketFD = -1;
	list<int> listValue;
	string strSql;
	string strLog;

	while (1)
	{
		tdEnquireLink->threadSleep(10);

		/** Check Enquire link response **/
		if (vEnquireLink.size())
		{
			/** Close socket that doesn't deliver enquire link response within 10 seconds **/
			for (vector<int>::iterator it = vEnquireLink.begin(); it != vEnquireLink.end(); ++it)
			{
				strSql = "DELETE FROM controller WHERE socket_fd = " + ConvertToString(*it) + ";";
				sqlite->controllerSqlExec(strSql.c_str());
				close(*it);
				_log("[Controller] Dropped connection, Close socket file descriptor filedes: %d", *it);
			}
		}
		vEnquireLink.clear();

		if (0 < getBindSocket(listValue))
		{
			for (list<int>::iterator i = listValue.begin(); i != listValue.end(); ++i)
			{
				nSocketFD = *i;
				vEnquireLink.push_back(nSocketFD);
				cmpEnquireLinkRequest(nSocketFD);
			}
		}

		listValue.clear();
	}
}

int CController::cmpEnquireLinkRequest(const int nSocketFD)
{
	return sendCommand(nSocketFD, enquire_link_request, STATUS_ROK, getSequence(), false, NULL);
}

int CController::getBindSocket(list<int> &listValue)
{
	string strSql = "SELECT socket_fd FROM controller WHERE status = 1;";
	return sqlite->getControllerColumeValueInt(strSql.c_str(), listValue, 0);
}

/************************************* thread function **************************************/
void *threadEnquireLinkRequest(void *argv)
{
	CController* ss = reinterpret_cast<CController*>(argv);
	ss->runEnquireLinkRequest();
	return NULL;
}
