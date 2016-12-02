/*
 * CControlCenter.cpp
 *
 *  Created on: 2015年10月20日
 *      Author: Louis Ju
 */

#include <list>
#include <ctime>
#include "common.h"
#include "CSocketServer.h"
#include "event.h"
#include "packet.h"
#include "CControlCenter.h"
#include "utility.h"
#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "CSqliteHandler.h"
#include "CThreadHandler.h"
#include "CAccessLog.h"
#include "CInitial.h"
#include "CSignup.h"
#include "CAuthentication.h"

#include "config.h"

using namespace std;

static CControlCenter * controlcenter = 0;

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

CControlCenter::CControlCenter() :
		CObject(), cmpServer(new CSocketServer), cmpParser(CCmpHandler::getInstance()), sqlite(
				CSqliteHandler::getInstance()), tdEnquireLink(new CThreadHandler), tdExportLog(new CThreadHandler), accessLog(
				CAccessLog::getInstance()), authentication(CAuthentication::getInstance())
{
	for (int i = 0; i < MAX_FUNC_POINT; ++i)
	{
		cmpRequest[i] = &CControlCenter::cmpUnknow;
	}
	cmpRequest[bind_request] = &CControlCenter::cmpBind;
	cmpRequest[unbind_request] = &CControlCenter::cmpUnbind;
	cmpRequest[power_port_set_request] = &CControlCenter::cmpPowerPort;
	cmpRequest[power_port_state_request] = &CControlCenter::cmpPowerPortState;
	cmpRequest[access_log_request] = &CControlCenter::cmpAccessLog;
	cmpRequest[initial_request] = &CControlCenter::cmpInitial;
	cmpRequest[sign_up_request] = &CControlCenter::cmpSignup;
	cmpRequest[sdk_tracker_request] = &CControlCenter::cmpSdkTracker;
	cmpRequest[authentication_request] = &CControlCenter::cmpAuthentication;
}

CControlCenter::~CControlCenter()
{
	delete cmpParser;
}

CControlCenter* CControlCenter::getInstance()
{
	if (0 == controlcenter)
	{
		controlcenter = new CControlCenter();
	}
	return controlcenter;
}

BOOL CControlCenter::startMongo(const std::string strIP, const int nPort)
{
	if (-1 != accessLog->connectDB(strIP, nPort))
	{
		_log("[Center] Connect Mongodb Controller Success");
		return TRUE;
	}
	else
	{
		_log("[Center] Connect Mongodb Controller Fail");
	}
	return FALSE;
}

BOOL CControlCenter::startSqlite(const int nDBId, const std::string strDB)
{
	if (strDB.empty())
		return FALSE;

	mkdirp(strDB);

	switch (nDBId)
	{
		case ID_DB_CONTROLLER:
			if (!sqlite->openControllerDB(strDB.c_str()))
			{
				_log("[Center] Open Sqlite DB controller fail");
				return FALSE;
			}
			break;
		case ID_DB_IDEAS:
			if (!sqlite->openIdeasDB(strDB.c_str()))
			{
				_log("[Center] Open Sqlite DB ideas fail");
				return FALSE;
			}
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

void CControlCenter::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
		case EVENT_COMMAND_SOCKET_CONTROL_CENTER_RECEIVE:
			onCMP(nId, nDataLen, pData);
			break;
		case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT:
			_log("[Center] Socket Client FD:%d Close", (int) nId);
			break;
		default:
			_log("[Center] Unknow message command: %d", nCommand);
			break;
	}
}

int CControlCenter::startServer(const int nPort)
{
	if (0 >= nPort)
		return FALSE;
	/** Run socket server for CMP **/
	cmpServer->setPackageReceiver( MSG_ID, EVENT_FILTER_CONTROL_CENTER, EVENT_COMMAND_SOCKET_CONTROL_CENTER_RECEIVE);
	cmpServer->setClientDisconnectCommand( EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT);

	if ( FAIL == cmpServer->start( AF_INET, NULL, nPort))
	{
		_DBG("CMP Server Socket Create Fail");
		return FALSE;
	}

	tdEnquireLink->createThread(threadEnquireLinkRequest, this);

	return TRUE;
}

void CControlCenter::stopServer()
{
	if (tdEnquireLink)
	{
		tdEnquireLink->threadExit();
		delete tdEnquireLink;
		_DBG("[Center] Stop Enquire Link Thread");
	}

	if (cmpServer)
	{
		cmpServer->stop();
		delete cmpServer;
		cmpServer = 0;
	}
}

int CControlCenter::sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp)
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

void CControlCenter::ackPacket(int nClientSocketFD, int nCommand, const void * pData)
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

int CControlCenter::cmpUnknow(int nSocket, int nCommand, int nSequence, const void * pData)
{
	_DBG("[Center] Unknow command:%d", nCommand);
	sendCommand(nSocket, nCommand, STATUS_RINVCMDID, nSequence, true);
	return 0;
}

int CControlCenter::cmpBind(int nSocket, int nCommand, int nSequence, const void * pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet)
	{
		_DBG("[Center] Bind Get Controller ID:%s Socket FD:%d", rData["id"].c_str(), nSocket);
		string strSql = "DELETE FROM controller WHERE id = '" + rData["id"] + "';";
		nRet = sqlite->controllerSqlExec(strSql.c_str());

		if ( SUCCESS == nRet)
		{
			const string strSocketFD = ConvertToString(nSocket);
			strSql = "INSERT INTO controller(id, status, socket_fd, created_date)values('" + rData["id"] + "',1,"
					+ strSocketFD + ",datetime());";
			nRet = sqlite->controllerSqlExec(strSql.c_str());
			if ( SUCCESS == nRet)
			{
				sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true);
				rData.clear();
				return nRet;
			}
		}
	}

	_DBG("[Center] Bind Fail, Invalid Controller ID Socket FD:%d", nSocket);
	sendCommand(nSocket, nCommand, STATUS_RINVCTRLID, nSequence, true);
	rData.clear();

	return FAIL;
}

int CControlCenter::cmpUnbind(int nSocket, int nCommand, int nSequence, const void * pData)
{
	sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true);
	return 0;
}

int CControlCenter::getControllerSocketFD(std::string strControllerID)
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

int CControlCenter::cmpPowerPort(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("wire") && rData.isValidKey("port") && rData.isValidKey("state")
			&& rData.isValidKey("controller"))
	{
		sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true);
		_DBG("[Center] Power Port Setting Wire:%s Port:%s state:%s Controller:%s Socket FD:%d", rData["wire"].c_str(),
				rData["port"].c_str(), rData["state"].c_str(), rData["controller"].c_str(), nSocket);
		int nFD = getControllerSocketFD(rData["controller"]);
		if (0 < nFD)
		{
			_DBG("[Center] Get Socket FD:%d Controller ID:%s", nFD, rData["controller"].c_str());
			cmpPowerPortRequest(nFD, rData["wire"], rData["port"], rData["state"]);
		}
		else
		{
			_DBG("[Center] Get Socket FD Fail Controller ID:%s", rData["controller"].c_str());
		}
	}
	else
	{
		_DBG("[Center] Power Port Setting Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();

	return 0;
}

int CControlCenter::cmpPowerPortState(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("wire") && rData.isValidKey("controller"))
	{
		_DBG("[Center] Power Port State Request Wire:%s Controller:%s Socket FD:%d", rData["wire"].c_str(),
				rData["controller"].c_str(), nSocket);
		int nFD = getControllerSocketFD(rData["controller"]);
		if (0 < nFD)
		{
			_DBG("[Center] Get Socket FD:%d Controller ID:%s", nFD, rData["controller"].c_str());
			if (0 < cmpPowerPortStateRequest(nFD, rData["wire"]))
			{
				cmpPowerPortStateResponse(nSocket, nSequence,
						"{\"count\":1,\"wires\":[{\"wire\":1,\"state\": \"1111\"}]}");
			}
			else
			{
				sendCommand(nSocket, nCommand, STATUS_RPPSTAFAIL, nSequence, true);
				_DBG("[Center] Get Power Port State Fail Controller ID:%s", rData["controller"].c_str());
			}
		}
		else
		{
			sendCommand(nSocket, nCommand, STATUS_RSYSERR, nSequence, true);
			_DBG("[Center] Get Socket FD Fail Controller ID:%s", rData["controller"].c_str());
		}
	}
	else
	{
		_DBG("[Center] Power Port Setting Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();

	return 0;
}

int CControlCenter::cmpAccessLog(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("type") && rData.isValidKey("data"))
	{
		_log("[Center] cmpAccessLog Receive Body: type=%s data=%s", rData["type"].c_str(), rData["data"].c_str());

		int nType = -1;
		convertFromString(nType, rData["type"]);
		accessLog->cmpAccessLogRequest(rData["type"], rData["data"]);
	}
	else
	{
		_DBG("[Center] Access Log Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return 0;
}

int CControlCenter::cmpInitial(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("type"))
	{
		_log("[Center] Receive Body: type=%s ", rData["type"].c_str());
		CInitial *init = new CInitial();
		int nType = 0;
		convertFromString(nType, rData["type"]);
		string strData = init->getInitData(nType);
		if (strData.empty())
		{
			_log("[Center] Initial Fail, Can't get initial data Socket FD:%d", nSocket);
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
		_log("[Center] Initial Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return nRet;
}

int CControlCenter::cmpAuthentication(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("type") && rData.isValidKey("data"))
	{
		_log("[Center] Receive Body: type=%s data=%s", rData["type"].c_str(), rData["data"].c_str());
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
			sendCommand(nSocket, nCommand, STATUS_RAUTHFAIL, nSequence, true);
		}
	}
	else
	{
		_log("[Center] Authentication Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return nRet;
}

int CControlCenter::cmpSdkTracker(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("data"))
	{
		sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true);

	}
	else
	{
		_log("[Center] SDK Tracker Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return nRet;
}

int CControlCenter::cmpSignup(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("type") && rData.isValidKey("data"))
	{
		_log("[Center] cmpSignup Body: type=%s data=%s", rData["type"].c_str(), rData["data"].c_str());

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
		_log("[Center] Sign up Fail, Invalid Body Parameters Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return nRet;
}

int CControlCenter::cmpPowerPortStateResponse(int nSocket, int nSequence, const char * szData)
{
	return cmpResponse(nSocket, power_port_state_response, nSequence, szData);
}

int CControlCenter::cmpInitialResponse(int nSocket, int nSequence, const char * szData)
{
	return cmpResponse(nSocket, initial_response, nSequence, szData);
}

int CControlCenter::cmpMdmLoginResponse(int nSocket, int nSequence, const char * szData)
{
	return cmpResponse(nSocket, mdm_login_response, nSequence, szData);
}

int CControlCenter::cmpResponse(const int nSocket, const int nCommandId, const int nSequence, const char * szData)
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
	printPacket(nCommandId, STATUS_ROK, nSequence, nRet, "[Center] cmpResponse", nSocket);

	string strLog;
	if (0 >= nRet)
	{
		_log("[Center] cmpResponse Fail socket: %d", nSocket);
	}

	return nRet;
}

int CControlCenter::cmpPowerPortRequest(int nSocket, std::string strWire, std::string strPort, std::string strState)
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

int CControlCenter::cmpPowerPortStateRequest(int nSocket, std::string strWire)
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

void CControlCenter::onCMP(int nClientFD, int nDataLen, const void *pData)
{
	_DBG("[Center] Receive CMP From Client:%d Length:%d", nClientFD, nDataLen);

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

void CControlCenter::runEnquireLinkRequest()
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
				_log("[Center] Dropped connection, Close socket file descriptor filedes: %d", *it);
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

int CControlCenter::cmpEnquireLinkRequest(const int nSocketFD)
{
	return sendCommand(nSocketFD, enquire_link_request, STATUS_ROK, getSequence(), false);
}

int CControlCenter::getBindSocket(list<int> &listValue)
{
	string strSql = "SELECT socket_fd FROM controller WHERE status = 1;";
	return sqlite->getControllerColumeValueInt(strSql.c_str(), listValue, 0);
}

/************************************* thread function **************************************/
void *threadEnquireLinkRequest(void *argv)
{
	CControlCenter* ss = reinterpret_cast<CControlCenter*>(argv);
	ss->runEnquireLinkRequest();
	return NULL;
}
