/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include <list>
#include <ctime>

#include "CAccessLog.h"
#include "CAuthentication.h"
#include "CInitial.h"
#include "CSignup.h"
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

using namespace std;

static CController * controller = 0;

/** Enquire link function declare for enquire link thread **/
void *threadEnquireLinkRequest(void *argv)
{
	CController* ss = reinterpret_cast<CController*>(argv);
	ss->runEnquireLinkRequest();
	return NULL;
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

CController::CController() :
		CObject(), cmpServer(new CSocketServer), cmpParser(CCmpHandler::getInstance()), sqlite(
				CSqliteHandler::getInstance()), tdEnquireLink(new CThreadHandler), tdExportLog(new CThreadHandler), accessLog(
				CAccessLog::getInstance()), authentication(CAuthentication::getInstance()), cmpClient(new CSocketClient)
{
	for (int i = 0; i < MAX_FUNC_POINT; ++i)
	{
		cmpRequest[i] = &CController::cmpUnknow;
	}
	cmpRequest[bind_request] = &CController::cmpBind;
	cmpRequest[unbind_request] = &CController::cmpUnbind;
	cmpRequest[power_port_set_request] = &CController::cmpPowerPort;
	cmpRequest[power_port_state_request] = &CController::cmpPowerPortState;
	cmpRequest[access_log_request] = &CController::cmpAccessLog;
	cmpRequest[initial_request] = &CController::cmpInitial;
	cmpRequest[sign_up_request] = &CController::cmpSignup;
	cmpRequest[authentication_request] = &CController::cmpAuthentication;
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

BOOL CController::startMongo(const std::string strIP, const int nPort)
{
	if (-1 != accessLog->connectDB(strIP, nPort))
	{
		_log("[Controller] Connect Mongodb Controller Success");
		return TRUE;
	}
	else
	{
		_log("[Controller] Connect Mongodb Controller Fail");
	}
	return FALSE;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_RECEIVE:
		onCMP(nId, nDataLen, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT:
		_log("[Controller] Socket Client FD:%d Connected", (int) nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT:
		_log("[Controller] Socket Client FD:%d Close", (int) nId);
		break;
	default:
		_log("[Controller] Unknow message command: %d", nCommand);
		break;
	}
}

int CController::startServer(const int nPort, const int nMsqId)
{
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if (0 < nMsqId)
	{
		cmpServer->setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_RECEIVE);
		cmpServer->setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT);
		cmpServer->setClientDisconnectCommand( EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT);
	}

	if ( FAIL == cmpServer->start( AF_INET, NULL, nPort))
	{
		_log("CMP Server Socket Create Fail");
		return FALSE;
	}

	tdEnquireLink->createThread(threadEnquireLinkRequest, this);

	return TRUE;
}

void CController::stopServer()
{
	if (tdEnquireLink)
	{
		tdEnquireLink->threadExit();
		delete tdEnquireLink;
		_DBG("[Controller] Stop Enquire Link Thread");
	}

	if (cmpServer)
	{
		cmpServer->stop();
		delete cmpServer;
		cmpServer = 0;
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
	nRet = cmpServer->socketSend(nSocket, &cmpHeader, sizeof(CMP_HEADER));
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

int CController::cmpPowerPort(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("wire") && rData.isValidKey("port") && rData.isValidKey("state")
			&& rData.isValidKey("controller"))
	{
		sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true);
		_DBG("[Controller] Power Port Setting Wire:%s Port:%s state:%s Controller:%s Socket FD:%d",
				rData["wire"].c_str(), rData["port"].c_str(), rData["state"].c_str(), rData["controller"].c_str(),
				nSocket);
		int nFD = getControllerSocketFD(rData["controller"]);
		if (0 < nFD)
		{
			_DBG("[Controller] Get Socket FD:%d Controller ID:%s", nFD, rData["controller"].c_str());
			cmpPowerPortRequest(nFD, rData["wire"], rData["port"], rData["state"]);
		}
		else
		{
			_DBG("[Controller] Get Socket FD Fail Controller ID:%s", rData["controller"].c_str());
		}
	}
	else
	{
		_DBG("[Controller] Power Port Setting Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();

	return 0;
}

int CController::cmpPowerPortState(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("wire") && rData.isValidKey("controller"))
	{
		_DBG("[Controller] Power Port State Request Wire:%s Controller:%s Socket FD:%d", rData["wire"].c_str(),
				rData["controller"].c_str(), nSocket);
		int nFD = getControllerSocketFD(rData["controller"]);
		if (0 < nFD)
		{
			_DBG("[Controller] Get Socket FD:%d Controller ID:%s", nFD, rData["controller"].c_str());
			if (0 < cmpPowerPortStateRequest(nFD, rData["wire"]))
			{
				cmpPowerPortStateResponse(nSocket, nSequence,
						"{\"count\":1,\"wires\":[{\"wire\":1,\"state\": \"1111\"}]}");
			}
			else
			{
				sendCommand(nSocket, nCommand, STATUS_RSYSERR, nSequence, true);
				_DBG("[Controller] Get Power Port State Fail Controller ID:%s", rData["controller"].c_str());
			}
		}
		else
		{
			sendCommand(nSocket, nCommand, STATUS_RSYSERR, nSequence, true);
			_DBG("[Controller] Get Socket FD Fail Controller ID:%s", rData["controller"].c_str());
		}
	}
	else
	{
		_DBG("[Controller] Power Port Setting Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();

	return 0;
}

int CController::cmpAccessLog(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("type") && rData.isValidKey("data"))
	{
		_log("[Controller] cmpAccessLog Receive Body: type=%s data=%s", rData["type"].c_str(), rData["data"].c_str());

		int nType = -1;
		convertFromString(nType, rData["type"]);
		accessLog->cmpAccessLogRequest(rData["type"], rData["data"]);
	}
	else
	{
		_DBG("[Controller] Access Log Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return 0;
}

int CController::cmpInitial(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("type"))
	{
		_log("[Controller] Receive Body: type=%s ", rData["type"].c_str());
		CInitial *init = new CInitial();
		int nType = 0;
		convertFromString(nType, rData["type"]);
		string strData = init->getInitData(nType);
		if (strData.empty())
		{
			_log("[Controller] Initial Fail, Can't get initial data Socket FD:%d", nSocket);
			sendCommand(nSocket, nCommand, STATUS_RSYSERR, nSequence, true);
		}
		else
		{
			cmpInitialResponse(nSocket, nSequence, strData.c_str());
		}
		delete init;
	}
	else
	{
		_log("[Controller] Initial Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return nRet;
}

int CController::cmpAuthentication(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("type") && rData.isValidKey("data"))
	{
		_log("[Controller] Receive Body: type=%s data=%s", rData["type"].c_str(), rData["data"].c_str());
		int nType = -1;
		bool bAuth = false;
		convertFromString(nType, rData["type"]);
		bAuth = authentication->authorization(nType, rData["data"]);
		if (bAuth)
		{
			sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true);
		}
		else
		{
			sendCommand(nSocket, nCommand, STATUS_RSYSERR, nSequence, true);
		}
	}
	else
	{
		_log("[Controller] Authentication Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return nRet;
}

int CController::cmpSdkTracker(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("data"))
	{
		sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true);

	}
	else
	{
		_log("[Controller] SDK Tracker Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return nRet;
}

int CController::cmpSignup(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("type") && rData.isValidKey("data"))
	{
		_log("[Controller] cmpSignup Body: type=%s data=%s", rData["type"].c_str(), rData["data"].c_str());

		CSignup *signup = new CSignup();

		if (INSERT_FAIL != signup->insert(rData["data"]))
		{
			sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true);
		}
		else
		{
			sendCommand(nSocket, nCommand, STATUS_RSYSERR, nSequence, true);
		}
		delete signup;
	}
	else
	{
		_log("[Controller] Sign up Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return nRet;
}

int CController::cmpPowerPortStateResponse(int nSocket, int nSequence, const char * szData)
{
	return cmpResponse(nSocket, power_port_state_response, nSequence, szData);
}

int CController::cmpInitialResponse(int nSocket, int nSequence, const char * szData)
{
	return cmpResponse(nSocket, initial_response, nSequence, szData);
}

int CController::cmpMdmLoginResponse(int nSocket, int nSequence, const char * szData)
{
	return cmpResponse(nSocket, rdm_login_response, nSequence, szData);
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

	nRet = cmpServer->socketSend(nSocket, &packet, nTotal_len);
	printPacket(nCommandId, STATUS_ROK, nSequence, nRet, "[Controller] cmpResponse", nSocket);

	string strLog;
	if (0 >= nRet)
	{
		_log("[Controller] cmpResponse Fail socket: %d", nSocket);
	}

	return nRet;
}

int CController::cmpPowerPortRequest(int nSocket, std::string strWire, std::string strPort, std::string strState)
{
	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	void *pHeader = &packet.cmpHeader;
	char *pIndex = packet.cmpBody.cmpdata;

	memset(&packet, 0, sizeof(CMP_PACKET));

	cmpParser->formatHeader( power_port_set_request, STATUS_ROK, getSequence(), &pHeader);

	memcpy(pIndex, strWire.c_str(), 1); // wire
	++pIndex;
	++nBody_len;

	memcpy(pIndex, strPort.c_str(), 1);	//	port
	++pIndex;
	++nBody_len;

	memcpy(pIndex, strState.c_str(), 1);	//	state
	++pIndex;
	++nBody_len;

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);

	nRet = cmpServer->socketSend(nSocket, &packet, nTotal_len);

	return nRet;
}

int CController::cmpPowerPortStateRequest(int nSocket, std::string strWire)
{
	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	void *pHeader = &packet.cmpHeader;
	char *pIndex = packet.cmpBody.cmpdata;

	memset(&packet, 0, sizeof(CMP_PACKET));

	cmpParser->formatHeader( power_port_state_request, STATUS_ROK, getSequence(), &pHeader);

	memcpy(pIndex, strWire.c_str(), 1); // wire
	++pIndex;
	++nBody_len;

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);

	nRet = cmpServer->socketSend(nSocket, &packet, nTotal_len);

	return nRet;
}

void CController::onCMP(int nClientFD, int nDataLen, const void *pData)
{
	_DBG("[Controller] Receive CMP From Client:%d Length:%d", nClientFD, nDataLen);

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

