/*
 * CController.cpp
 *
 *  Created on: 2015年10月19日
 *      Author: Louis Ju
 */

#include "CController.h"

#include "common.h"
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

CController::CController() :
		CObject(), trackerServer(CTrackerServer::getInstance()), mongodb(CMongoDBHandler::getInstance())
{

}

CController::~CController()
{
	delete mongodb;
	delete trackerServer;
}

CController* CController::getInstance()
{
	if(0 == controller)
	{
		controller = new CController();
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

int CController::startMongoClient()
{
	return mongodb->connectDB("175.98.119.121", "27017");
}

int CController::stop()
{
	trackerServer->stop();
	return FALSE;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{

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
	default:
		_log("[Mongodb Controller] Insert Access log fail, unknow service type: %d", nType);
		break;
	}
	return strOID;
}

