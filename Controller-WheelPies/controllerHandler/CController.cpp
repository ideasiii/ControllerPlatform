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

using namespace std;

CController::CController() :
		mnMsqKey(-1), mysql(0), cmpwheelpies(0)
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

	return nResult;
}

int CController::onFinish(void* nMsqKey)
{
	mysql->close();
	delete mysql;

	return TRUE;
}

void CController::onHandleMessage(Message &message)
{
	switch(message.what)
	{
	case wheelpies_request:
		// Lambda Expression

		break;
	}
}
