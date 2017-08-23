/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include <string>
#include "common.h"
#include "CController.h"
#include "CDispatcher.h"
#include "CConfig.h"
#include "event.h"
#include "utility.h"

using namespace std;

CController::CController() :
		mnMsqKey(-1), dispatcher(CDispatcher::getInstance())
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_DISPATCHER;
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nRet;
	int nPort;
	string strValue;
	CConfig *config;
	string strConfPath;

	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if(strConfPath.empty())
		return FALSE;

	nRet = FALSE;
	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		strValue = config->getValue("SERVER DISPATCHER", "port");
		if(!strValue.empty())
		{
			convertFromString(nPort, strValue);
			nRet = startDispatcher(nPort, mnMsqKey);
			if(nRet)
			{
				strValue = config->getValue("SERVER SIGNIN", "port");
				convertFromString(nPort, strValue);
				dispatcher->addServer(ID_SERVER_SIGNIN, config->getValue("SERVER SIGNIN", "name").c_str(),
						config->getValue("SERVER SIGNIN", "ip").c_str(), nPort);

				strValue = config->getValue("SERVER TRACKER", "port");
				convertFromString(nPort, strValue);
				dispatcher->addServer(ID_SERVER_TRACKER, config->getValue("SERVER TRACKER", "name").c_str(),
						config->getValue("SERVER TRACKER", "ip").c_str(), nPort);

				dispatcher->createResp();
			}
		}
	}
	delete config;
	return nRet;
}

int CController::onFinish(void* nMsqKey)
{
	dispatcher->stop();
	delete dispatcher;
	return TRUE;
}

int CController::startDispatcher(const int nPort, int nMsqKey)
{
	if(dispatcher->start(0, nPort, nMsqKey))
	{
		dispatcher->idleTimeout(true, 3);
		return TRUE;
	}
	return FALSE;
}
