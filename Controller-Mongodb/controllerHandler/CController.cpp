/*
 * CController.cpp
 *
 *  Created on: 2015年10月19日
 *      Author: Louis Ju
 */

#include <string>
#include <map>
#include "CController.h"
#include "common.h"
#include "event.h"
#include "packet.h"
#include "utility.h"
#include "CConfig.h"
#include "CMongoDBHandler.h"
#include "CTrackerServer.h"

using namespace std;

#define ID_TIMER_MONGO_CONN			27017

CController::CController() :
		trackerServer(0), mongodb(0), mnMsqKey(-1)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	trackerServer = new CTrackerServer(this);
	mongodb = CMongoDBHandler::getInstance();
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_MONGODB;
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nRet;
	int nPort;
	CConfig *config;
	string strConfPath;
	string strValue;

	nRet = FALSE;
	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if(strConfPath.empty())
		return FALSE;

	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		mongo_conn.strIP = config->getValue("MONGODB", "ip");
		mongo_conn.strPort = config->getValue("MONGODB", "port");
		if(!startMongoClient(mongo_conn.strIP.c_str(), mongo_conn.strPort.c_str()))
		{
			_log("[CController] onInitial MongoDB Connect Fail!!");
			setTimer(ID_TIMER_MONGO_CONN, 5, 5);
		}

		strValue = config->getValue("SERVER TRACKER", "port");
		if(!strValue.empty())
		{
			convertFromString(nPort, strValue);
			nRet = startTrackerServer(nPort, mnMsqKey);
		}
	}

	delete config;
	return nRet;
}

void CController::onTimer(int nId)
{
	switch(nId)
	{
	case ID_TIMER_MONGO_CONN:
		killTimer(ID_TIMER_MONGO_CONN);
		_log("[CController] onTimer run MongoDB reconnect");
		if(!startMongoClient(mongo_conn.strIP.c_str(), mongo_conn.strPort.c_str()))
		{
			_log("[CController] onInitial MongoDB Connect Fail!!");
			setTimer(ID_TIMER_MONGO_CONN, 5, 5);
		}
		break;
	}
}

int CController::onFinish(void* nMsqKey)
{
	trackerServer->stop();
	delete mongodb;
	delete trackerServer;
	return TRUE;
}

void CController::onHandleMessage(Message &message)
{
	switch(message.what)
	{
	case access_log_request:
		insertLog(TYPE_MOBILE_SERVICE, message.strData);
		break;
	}
}

int CController::startTrackerServer(const int nPort, const int nMsqId)
{
	if(trackerServer->start(0, nPort, nMsqId))
		return TRUE;
	return FALSE;
}

int CController::startMongoClient(const char *szIP, const char *szPort)
{
	return mongodb->connectDB(szIP, szPort);
}

string CController::insertLog(const int nType, string strData)
{
	string strOID;

	string strJSON = trim(strData);
	strJSON = ReplaceAll(strJSON, "\"{", "{");
	strJSON = ReplaceAll(strJSON, "}\"", "}");
	strJSON = ReplaceAll(strJSON, "\"[", "[");
	strJSON = ReplaceAll(strJSON, "]\"", "]");
	strJSON = ReplaceAll(strJSON, "\\\"", "\"");

	if(mongodb->isValid())
	{
		switch(nType)
		{
		case TYPE_MOBILE_SERVICE:
			strOID = mongodb->insert("access", "mobile", strJSON);
			break;
		case TYPE_POWER_CHARGE_SERVICE:
			strOID = mongodb->insert("access", "power", strJSON);
			break;
		case TYPE_SDK_SERVICE:
			strOID = mongodb->insert("access", "sdk", strJSON);
			break;
		case TYPE_TRACKER_SERVICE:
			strOID = mongodb->insert("access", "tracker", strJSON);
			break;
		case TYPE_TRACKER_APPLIENCE:
			strOID = mongodb->insert("access", "applience", strJSON);
			break;
		case TYPE_TRACKER_TOY:
			strOID = mongodb->insert("access", "toy", strJSON);
			break;
		case TYPE_TRACKER_IOT:
			strOID = mongodb->insert("access", "iot", strJSON);
			break;
		default:
			_log("[CController] insertLog Insert Access log fail, unknow service type: %d", nType);
			break;
		}
	}
	else
	{
		_log("[CController] insertLog fail MongoDB Invalid");
		setTimer(ID_TIMER_MONGO_CONN, 5, 5);
	}
	return strOID;
}

