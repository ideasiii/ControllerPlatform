/*
 * CController.cpp
 *
 *  Created on: 2015年10月19日
 *      Author: Louis Ju
 */

#include "CController.h"

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
#include "CTrackerServer.h"

using namespace std;

static CController * controller = 0;

CController::Controller() :
		CObject(), cmpServer(new CSocketServer), cmpParser(CCmpHandler::getInstance()), mongodb(
				CMongoDBHandler::getInstance()), trackerServer(CTrackerServer::getInstance())
{

}

CController::~Controller()
{
	delete mongodb;
	delete cmpParser;
}

CController* Controller::getInstance()
{
	if(0 == controller)
	{
		controller = new Controller();
	}
	return controller;
}

int CController::startTrackerServer(const char *szIP, const int nPort)
{
	if(trackerServer->start(szIP, nPort))
	{
		return TRUE;
	}
	return FALSE;
}

int CController::stop()
{
	trackerServer->stop();
	return FALSE;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{

}

int CController::startServer(const int nPort)
{
	/** Run socket server for CMP **/
	cmpServer->setPackageReceiver(MSG_ID, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_SERVER_RECEIVE);
	if(0 >= nPort)
	{
		_log("Mongodb Controller Start Fail, Invalid Port:%d", nPort);
		return FALSE;
	}
	/** Start TCP/IP socket listen **/
	if(FAIL == cmpServer->start(AF_INET, NULL, nPort))
	{
		_log("Mongodb Controller Socket Server Create Fail");
		return FALSE;
	}

	mongodb->connectDB("127.0.0.1", "27017");
	return TRUE;
}

int CController::cmpUnknow(int nSocket, int nCommand, int nSequence, const void * pData)
{
	_DBG("[Mongodb Controller] Unknow command:%d", nCommand);
	sendCommandtoClient(nSocket, nCommand, STATUS_RINVCMDID, nSequence, true);
	return 0;
}

int CController::cmpAccessLog(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if(0 < nRet && rData.isValidKey("data"))
	{
		string strOID = insertLog(TYPE_MOBILE_SERVICE, rData["data"]);
		if(!strOID.empty())
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

std::string CController::insertLog(const int nType, std::string strData)
{
	string strOID;
	switch(nType)
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
void CController::onClientCMP(int nClientFD, int nDataLen, const void *pData)
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

	if(access_log_request == cmpHeader.command_id)
	{
		cmpAccessLog(nClientFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);
		return;
	}
}
