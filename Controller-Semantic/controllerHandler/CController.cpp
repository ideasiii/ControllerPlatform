/*
 * CController.cpp
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */
#include <stdio.h>
#include "CController.h"
#include "CCmpWord.h"
#include "common.h"
#include "CConfig.h"
#include "utility.h"
#include "event.h"
#include <string>

using namespace std;

CController::CController() :
		cmpword(0), mnMsqKey(-1)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_SEMANTIC; //*(reinterpret_cast<int*>(nMsqKey));
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	string strConfPath = reinterpret_cast<const char*>(szConfPath);
	_DBG("[CController] onInitial Config File: %s", strConfPath.c_str());
	if(strConfPath.empty())
		return FALSE;

	int nPort;
	string strPort;
	CConfig *config;
	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		strPort = config->getValue("SERVER WORD", "port");
		if(!strPort.empty())
		{
			convertFromString(nPort, strPort);
			startCmpWordServer(nPort, mnMsqKey);
		}
	}
	delete config;
	return TRUE;
}

int CController::onFinish(void* nMsqKey)
{
	if(0 != cmpword)
	{
		delete cmpword;
		cmpword = 0;
	}
	return TRUE;
}

int CController::startCmpWordServer(int nPort, int nMsqKey)
{
	int nResult = FALSE;

	if(0 != cmpword)
	{
		delete cmpword;
		cmpword = 0;
	}

	cmpword = new CCmpWord();
	cmpword->start(0, nPort, nMsqKey);
	nResult = TRUE;

	return nResult;
}
