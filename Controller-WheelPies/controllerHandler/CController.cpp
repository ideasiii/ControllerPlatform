/*
 * CController.cpp
 *
 *  Created on: 2018年7月4日
 *      Author: Jugo
 */
#include <string>
#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include "CController.h"
#include "common.h"
#include "CConfig.h"
#include "utility.h"
#include "event.h"
#include "Handler.h"
#include "CObject.h"
#include "packet.h"
#include "JSONObject.h"

#include "CMysqlHandler.h"
#include "CString.h"
#include "CCmpWheelpies.h"
#include "CMongoDBHandler.h"

using namespace std;

CController::CController() :
		mnMsqKey(-1), mysql(0), cmpwheelpies(0), mongodb(0)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_WHEELPIES;
	mysql = new CMysqlHandler();
	cmpwheelpies = new CCmpWheelpies(this);
	mongodb = CMongoDBHandler::getInstance();
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nResult;
	int nCount;
	int nPort;
	string strConfPath;
	string strPort;
	CConfig *config;

	nResult = FALSE;
	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if(strConfPath.empty())
		return nResult;

	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		strPort = config->getValue("SERVER WHEELPIES", "port");
		if(!strPort.empty())
		{
			convertFromString(nPort, strPort);
			nResult = cmpwheelpies->start(0, nPort, mnMsqKey);
		}
	}
	delete config;

//	mysql->connect("127.0.0.1", "edubot", "edubot", "ideas123!", "5");
	mongodb->connectDB("127.0.0.1", "27017");
	return nResult;
}

int CController::onFinish(void* nMsqKey)
{
	mysql->close();
	delete mysql;

	delete mongodb;
	return TRUE;
}

void CController::onHandleMessage(Message &message)
{
	switch(message.what)
	{
	case wheelpies_request:
		insertData(message.strData);
		break;
	}
}

void CController::insertData(std::string strData)
{
	string strOID;

	string strJSON = trim(strData);
	strJSON = ReplaceAll(strJSON, "\"{", "{");
	strJSON = ReplaceAll(strJSON, "}\"", "}");
	strJSON = ReplaceAll(strJSON, "\"[", "[");
	strJSON = ReplaceAll(strJSON, "]\"", "]");
	strJSON = ReplaceAll(strJSON, "\\\"", "\"");

	strOID = mongodb->insert("sport", "wheelpies", strJSON);
}
