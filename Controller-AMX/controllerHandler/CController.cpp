/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include <list>
#include <ctime>

#include "common.h"
#include "CSocketServer.h"
#include "CSocketClient.h"
#include "event.h"
#include "packet.h"
#include "utility.h"
#include "CCmpHandler.h"
#include "CController.h"
#include "CDataHandler.cpp"
#include "CSqliteHandler.h"
#include "CThreadHandler.h"
#include "CServerAMX.h"
#include "CServerDevice.h"

using namespace std;

static CController * controller = 0;

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
	for (int i = 0; i < MAX_FUNC_POINT; ++i)
	{
		cmpRequest[i] = &CController::cmpUnknow;
	}
	cmpRequest[bind_request] = &CController::cmpBind;
	cmpRequest[unbind_request] = &CController::cmpUnbind;
	cmpRequest[device_control_request] = &CController::cmpDeviceControl;
	cmpRequest[device_state_request] = &CController::cmpDeviceState;
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
	case EVENT_COMMAND_SOCKET_TCP_RECEIVE:
		onCMP(nId, nDataLen, pData);
		break;
	case EVENT_COMMAND_SOCKET_TCP_AMX_RECEIVE:
		serverAMX->onReceive(nId, static_cast<char*>(const_cast<void*>(pData)));
		break;
	case EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE:
		onReceiveDevice(nId, static_cast<char*>(const_cast<void*>(pData)));
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT:
		_log("[Controller] Socket Client FD:%d Connected", (int) nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_AMX:
		_log("[Controller] AMX Socket Client FD:%d Connected", (int) nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE:
		_log("[Controller] Device Socket Client FD:%d Connected", (int) nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT:
		_log("[Controller] Socket Client FD:%d Close", (int) nId);
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

void CController::onReceiveDevice(const int nSocketFD, char * pCommand)
{
	int nLength = 0;
	string strCommand = pCommand;
	_log("[Controller] Receive Device Command: %s", strCommand.c_str());

	if (!strCommand.empty())
	{
		if (serverAMX)
		{
			nLength = serverAMX->sendCommand(strCommand);
			if (nLength == ((int) strCommand.length()))
			{
				serverDevice->sendCommand(nSocketFD, CTL_OK);
			}
			else
			{
				serverDevice->sendCommand(nSocketFD, CTL_ERROR);
			}
		}
		else
		{
			serverDevice->sendCommand(nSocketFD, CTL_ERROR);
			_log("[Controller] AMX Server Invalid");
		}
	}
	else
	{
		serverDevice->sendCommand(nSocketFD, CTL_ERROR);
		_log("[Controller] Device Send Invalid Command");
	}
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

int CController::sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp)
{
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
//	nRet = cmpServerAMX->socketSend(nSocket, &cmpHeader, sizeof(CMP_HEADER));
	printPacket(nCommandSend, nStatus, nSequence, nRet, "[Center Send]", nSocket);
	return nRet;
}

void CController::ackPacket(int nClientSocketFD, int nCommand, const void * pData)
{
	string strLog;
	switch (nCommand)
	{
	case generic_nack:
		break;
	case bind_response:
		break;
	case authentication_response:
		break;
	case access_log_response:
		break;
	case enquire_link_response:
		for (vector<int>::iterator it = vEnquireLink.begin(); it != vEnquireLink.end(); ++it)
		{
			if (nClientSocketFD == *it)
			{
				vEnquireLink.erase(it);
				break;
			}
		}
		break;
	case unbind_response:
		break;
	case update_response:
		break;
	case reboot_response:
		break;
	case config_response:
		break;
	case power_port_set_response:
		break;
	}
}

int CController::cmpUnknow(int nSocket, int nCommand, int nSequence, const void * pData)
{
	_DBG("[Controller] Unknow command:%d", nCommand);
	sendCommand(nSocket, nCommand, STATUS_RINVCMDID, nSequence, true);
	return 0;
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
				sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true);
				rData.clear();
				return nRet;
			}
		}
	}

	_log("[Controller] Bind Fail, Invalid Controller ID Socket FD:%d", nSocket);
	sendCommand(nSocket, nCommand, STATUS_RINVCTRLID, nSequence, true);
	rData.clear();

	return FAIL;
}

int CController::cmpUnbind(int nSocket, int nCommand, int nSequence, const void * pData)
{
	sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true);
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

int CController::cmpDeviceControl(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("item") && rData.isValidKey("value"))
	{
		_log("[Controller] cmpDeviceControl Body: item=%s value=%s", rData["item"].c_str(), rData["value"].c_str());
	}
	else
	{
		_log("[Controller] Device Control Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return nRet;
}

int CController::cmpDeviceState(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("item"))
	{
		_log("[Controller] cmpDeviceState Body: item=%s value=%s", rData["item"].c_str());
	}
	else
	{
		_log("[Controller] Device Control Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
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

void CController::onCMP(int nClientFD, int nDataLen, const void *pData)
{
	_log("[Controller] Receive CMP From Client:%d Length:%d Data:%s", nClientFD, nDataLen,
			static_cast<char*>(const_cast<void*>(pData)));

	/*
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
	 "[Center Recv]", nClientFD);

	 if (cmpParser->isAckPacket(cmpHeader.command_id))
	 {
	 ackPacket(nClientFD, cmpHeader.command_id, pPacket);
	 return;
	 }

	 if (0x000000FF < cmpHeader.command_id)
	 {
	 sendCommand(nClientFD, cmpHeader.command_id, STATUS_RINVCMDID, cmpHeader.sequence_number, true);
	 return;
	 }

	 (this->*this->cmpRequest[cmpHeader.command_id])(nClientFD, cmpHeader.command_id, cmpHeader.sequence_number,
	 pPacket);
	 */
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
	return sendCommand(nSocketFD, enquire_link_request, STATUS_ROK, getSequence(), false);
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
