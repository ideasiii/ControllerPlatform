/*
 * Controller.cpp
 *
 *  Created on: 2015年10月19日
 *      Author: Louis Ju
 */

#include "Controller.h"
#include "common.h"
#include "CConfig.h"
#include "CSocketServer.h"
#include "event.h"
#include "packet.h"
#include "CCmpHandler.h"
#include "utility.h"
#include "CDataHandler.cpp"
#include "IReceiver.h"
#include <map>
#include "CMongoDBHandler.h"
#include "LogHandler.h"
#include "packet.h"

using namespace std;

static Controller * controller = 0;

#define MSG_ID							27027
#define TYPE_TEST															20160604

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
	controller->onClientCMP(nSocketFD, nDataLen, pData);
	return 0;
}

Controller::Controller() :
		CObject(), cmpServer(new CSocketServer), cmpParser(CCmpHandler::getInstance()), mongodb(
				CMongoDBHandler::getInstance())
{

}

Controller::~Controller()
{
	delete mongodb;
	delete cmpParser;
}

Controller* Controller::getInstance()
{
	if (0 == controller)
	{
		controller = new Controller();
	}
	return controller;
}

int Controller::init(std::string strConf)
{
	/** Load config file **/
	CConfig *config = new CConfig();
	if ( FALSE == config->loadConfig(strConf))
	{
		_DBG("Load Config File Fail:%s", strConf.c_str());
		delete config;
		return FALSE;
	}

	/** Server init and start **/
	string strPort = config->getValue("SERVER", "port");
	delete config;

	if (!strPort.empty())
	{
		int nPort = -1;
		convertFromString(nPort, strPort);
		return startServer(nPort);
	}

	return FALSE;
}

void Controller::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_SERVER_RECEIVE:
		onClientCMP(nId, nDataLen, pData);
		break;
	default:
		//		printLog("unknow message command", "[Mongodb Controller]", G_LOG_PATH);
		break;
	}
}

int Controller::startServer(const int nPort)
{
	/** Run socket server for CMP **/
	cmpServer->setPackageReceiver( MSG_ID, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_SERVER_RECEIVE);
	if (0 >= nPort)
	{
		_log("Mongodb Controller Start Fail, Invalid Port:%d", nPort);
		return FALSE;
	}
	/** Start TCP/IP socket listen **/
	if ( FAIL == cmpServer->start( AF_INET, NULL, nPort))
	{
		_log("Mongodb Controller Socket Server Create Fail");
		return FALSE;
	}

	mongodb->connectDB("127.0.0.1", "27017");
	return TRUE;
}

void Controller::stopServer()
{
	if (cmpServer)
	{
		cmpServer->stop();
		delete cmpServer;
		cmpServer = 0;
	}
}

int Controller::sendCommandtoClient(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp)
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
//	printPacket(nCommandSend, nStatus, nSequence, nRet, "[Controller Send to Client]", G_LOG_PATH.c_str(), nSocket);
	return nRet;
}

int Controller::cmpUnknow(int nSocket, int nCommand, int nSequence, const void * pData)
{
	_DBG("[Mongodb Controller] Unknow command:%d", nCommand);
	sendCommandtoClient(nSocket, nCommand, STATUS_RINVCMDID, nSequence, true);
	return 0;
}

int Controller::cmpAccessLog(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("data"))
	{
		string strOID = insertLog(TYPE_MOBILE_SERVICE, rData["data"]);
		if (!strOID.empty())
		{
			_log("[Mongodb Controller] Insert DB Success, OID=%s: data=%s", strOID.c_str(), rData["data"].c_str());
		}
		else
		{
			_log("[Mongodb Controller] Insert DB Fail,  data=%s", rData["data"].c_str());
		}
	}
	else
	{
		_log("[Mongodb Controller] Access Log Fail, Invalid Body Parameters Socket FD:%d", nSocket);
	}
	rData.clear();
	return 0;
}

std::string Controller::insertLog(const int nType, std::string strData)
{
	string strOID;
	switch (nType)
	{
	case TYPE_MOBILE_SERVICE:
		strOID = mongodb->insert("access", "mobile", strData);
		break;
	case TYPE_POWER_CHARGE_SERVICE:
		strOID = mongodb->insert("access", "power", strData);
		break;
	case TYPE_SDK_SERVICE:
		strOID = mongodb->insert("access", "sdk", strData);
		break;
	case TYPE_TRACKER_SERVICE:
		strOID = mongodb->insert("access", "tracker", strData);
		break;
	case TYPE_TRACKER_APPLIENCE:
		strOID = mongodb->insert("access", "applience", strData);
		break;
	case TYPE_TRACKER_TOY:
		strOID = mongodb->insert("access", "toy", strData);
		break;
	case TYPE_TRACKER_IOT:
		strOID = mongodb->insert("access", "iot", strData);
		break;
	case TYPE_TEST:
		strOID = mongodb->insert("access", "test", strData);
		break;
	default:
		_log("[Mongodb Controller] Insert Access log fail, unknow service type: %d", nType);
		break;
	}
	return strOID;
}

/**
 * 	Receive CMP from Client
 */
void Controller::onClientCMP(int nClientFD, int nDataLen, const void *pData)
{
	_DBG("[Mongodb Controller] Receive CMP From Client:%d Length:%d", nClientFD, nDataLen);

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
			"[Mongodb Controller]", nClientFD);

	if (access_log_request == cmpHeader.command_id)
	{
		cmpAccessLog(nClientFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);
		return;
	}
}
