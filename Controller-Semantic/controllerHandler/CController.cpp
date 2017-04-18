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

using namespace std;

CController::CController() :
		cmpword(0)
{
	this->mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_SEMANTIC;
}

CController::~CController()
{

}

void CController::onInitial()
{
	startCmpWordServer();
}

void CController::onFinish()
{
	if(0 != cmpword)
	{
		delete cmpword;
		cmpword = 0;
	}
}

int CController::startCmpWordServer()
{
	int nPort;
	int nResult = FALSE;
	string strPort;
	string strConfPath;
	CConfig *config;

	strConfPath = getConfPath();

	if(strConfPath.empty())
	{
		_log("[CController] No config path");
		return FALSE;
	}

	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		strPort = config->getValue("SERVER WORD", "port");

		if(!strPort.empty())
		{
			if(0 != cmpword)
			{
				delete cmpword;
				cmpword = 0;
			}
			convertFromString(nPort, strPort);
			cmpword = new CCmpWord();
			cmpword->start(0, nPort, this->mnMsqKey);
			nResult = TRUE;
		}
	}
	else
	{
		_log("[CController] startCmpWordServer Fail, loadConfig Fail: %s", strConfPath.c_str());
	}
	delete config;

	return nResult;
}
