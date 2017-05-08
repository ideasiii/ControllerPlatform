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
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_SIGNIN;
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nRet;
	int nPort;
	string strIP;
	string strPort;
	CConfig *config;
	string strConfPath;

	nRet = FALSE;
	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if(strConfPath.empty())
		return FALSE;

	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		strIP = config->getValue("MONGODB", "ip");
		strPort = config->getValue("MONGODB", "port");
		nRet = startMongoClient(strIP.c_str(), strPort.c_str());

		if(nRet)
		{
			strPort = config->getValue("SERVER TRACKER", "port");
			if(!strPort.empty())
			{
				convertFromString(nPort, strPort);
				nRet = startTrackerServer(nPort, mnMsqKey);
			}
		}
	}

	delete config;
	return nRet;
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
	return trackerServer->start(0, nPort, nMsqId);
}

int CController::startMongoClient(const char *szIP, const char *szPort)
{
	return mongodb->connectDB(szIP, szPort);
}

string CController::insertLog(const int nType, string strData)
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
	default:
		_log("[Mongodb Controller] Insert Access log fail, unknow service type: %d", nType);
		break;
	}
	return strOID;
}

