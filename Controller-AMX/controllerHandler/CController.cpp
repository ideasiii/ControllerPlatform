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

using namespace std;

static CController * controller = 0;

/** Callback Function **/
void IonAMXCommand(void* param)
{
	string strParam = reinterpret_cast<const char*>(param);
	controller->onAMXCommand(strParam);
}

void CController::onAMXCommand(string strCommand)
{
	_log("[Controller] callback parameter: %s", strCommand.c_str());
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

CController::CController() :
		CObject(), serverAMX(CServerAMX::getInstance()), serverDevice(CServerDevice::getInstance()), cmpParser(
				CCmpHandler::getInstance()), sqlite(CSqliteHandler::getInstance()), tdEnquireLink(new CThreadHandler), tdExportLog(
				new CThreadHandler)
{
	mapFunc[amx_control_request] = &CController::cmpAmxControl;
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
		//onReceiveDevice(nId, static_cast<char*>(const_cast<void*>(pData)));
		serverDevice->onReceive(nId, pData, IonAMXCommand);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_AMX:
		_log("[Controller] AMX Socket Client FD:%d Connected", (int) nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE:
		_log("[Controller] Device Socket Client FD:%d Connected", (int) nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_AMX:
		_log("[Controller] AMX Socket Client FD:%d Close", (int) nId);
		serverAMX->unbind(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE:
		_log("[Controller] Device Socket Client FD:%d Close", (int) nId);
		break;
	default:
		_log("[Controller] Unknow message command: %d", nCommand);
		break;
	}
}

void CController::onReceiveDevice(const int nSocketFD, const void *pData)
{
	int nRet = -1;
	int nPacketLen = 0;
	CMP_HEADER cmpHeader;
	char *pPacket;

	pPacket = (char*) const_cast<void*>(pData);
	memset(&cmpHeader, 0, sizeof(CMP_HEADER));

	cmpHeader.command_id = cmpParser->getCommand(pPacket);
	cmpHeader.command_length = cmpParser->getLength(pPacket);
	cmpHeader.command_status = cmpParser->getStatus(pPacket);
	cmpHeader.sequence_number = cmpParser->getSequence(pPacket);

	printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
			"[Controller Recv]", nSocketFD);

	if (cmpParser->isAckPacket(cmpHeader.command_id))
	{
		return;
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if (0x000000FF < cmpHeader.command_id || 0x00000000 >= cmpHeader.command_id || mapFunc.end() == iter)
	{
		sendCommand(nSocketFD, cmpHeader.command_id, STATUS_RINVCMDID, cmpHeader.sequence_number, true,
				dynamic_cast<CSocket*>(serverDevice));
		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);

}

void CController::onReceiveAMX(const int nSocketFD, char * pCommand)
{
	string strCommand = pCommand;
	_log("[Controller] Receive AMX Command: %s", strCommand.c_str());

	if (0 == strCommand.substr(0, 4).compare("bind"))
	{
		serverAMX->bind(nSocketFD);
		return;
	}

	if (0 == strCommand.substr(0, 6).compare("unbind"))
	{
		serverAMX->unbind(nSocketFD);
		return;
	}

}

int CController::startServerAMX(const int nPort, const int nMsqId)
{
	return serverAMX->startServer(nPort, nMsqId);
}

int CController::startServerDevice(const int nPort, const int nMsqId)
{
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

int CController::sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp, CSocket *socket)
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

	cmpParser->formatHeader(nCommandSend, nStatus, nSequence, &pHeader);
	nRet = socket->socketSend(nSocket, &cmpHeader, sizeof(CMP_HEADER));
	printPacket(nCommandSend, nStatus, nSequence, nRet, "[Controller Send]", nSocket);
	return nRet;
}

int CController::cmpBind(int nSocket, int nCommand, int nSequence, const void * pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet)
	{
		_log("[Controller] Bind Get Controller ID:%s Socket FD:%d", rData["id"].c_str(), nSocket);
		string strSql = "DELETE FROM controller WHERE id = '" + rData["id"] + "';";
		nRet = sqlite->controllerSqlExec(strSql.c_str());

		if ( SUCCESS == nRet)
		{
			//const string strSocketFD = ConvertToString(nSocket);
			strSql = "INSERT INTO controller(id, status, socket_fd, created_date)values('" + rData["id"] + "',1,"
					+ ConvertToString(nSocket) + ",datetime());";
			nRet = sqlite->controllerSqlExec(strSql.c_str());
			if ( SUCCESS == nRet)
			{
				sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true, dynamic_cast<CSocket*>(serverAMX));
				rData.clear();
				return nRet;
			}
		}
	}

	_log("[Controller] Bind Fail, Invalid Controller ID Socket FD:%d", nSocket);
	sendCommand(nSocket, nCommand, STATUS_RINVCTRLID, nSequence, true, dynamic_cast<CSocket*>(serverAMX));
	rData.clear();

	return FAIL;
}

int CController::cmpUnbind(int nSocket, int nCommand, int nSequence, const void * pData)
{
	sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true, dynamic_cast<CSocket*>(serverAMX));
	return 0;
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

int CController::cmpAmxControl(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("data"))
	{
		_log("[Controller] cmpAmxControl Body: %s", rData["data"].c_str());

		/** get AMX string command **/
		JSONObject jobj(rData["data"].c_str());
		if (jobj.isValid())
		{
			int nFunction = jobj.getInt("function");
			int nDevice = jobj.getInt("device");
			int nControl = jobj.getInt("control");
			string strCommand = getAMXControl(nFunction, nDevice, nControl);
			if (!strCommand.empty())
			{
				_log("[Controller] AMX Command: %s", strCommand.c_str());
				sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true, dynamic_cast<CSocket*>(serverDevice));
				serverAMX->sendCommand(strCommand);
				return TRUE;
			}
		}
		_log("[Controller] cmpAmxControl Fail, Invalid JSON Data Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVJSON, nSequence, true, dynamic_cast<CSocket*>(serverDevice));
	}

	_log("[Controller] cmpAmxControl Fail, Invalid Body Parameters Socket FD:%d", nSocket);
	sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true, dynamic_cast<CSocket*>(serverDevice));

	rData.clear();
	return FALSE;
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
