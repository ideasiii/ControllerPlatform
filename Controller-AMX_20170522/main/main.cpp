/*
 * main.cpp
 *
 *  Created on: 2016年07月01日
 *      Author: Jugo
 */

#include <unistd.h>
#include <string.h>
#include <iostream>

#include "CController.h"
#include "CProcessHandler.h"
#include "CMessageHandler.h"
#include "event.h"
#include "LogHandler.h"
#include "CConfig.h"
#include "utility.h"
#include "common.h"
#include "CObject.h"

using namespace std;

string getConfName(string strProcessName);
void options(int argc, char **argv);
static void runService(int nKey);

int main(int argc, char* argv[])
{
	extern char *__progname;
	openlog(__progname, LOG_PID, LOG_LOCAL0);

	// Run Process
	//CProcessHandler::runProcess(runService);
	process(runService, EVENT_MSQ_KEY_CONTROLLER_AMX);
	closelog();
	return EXIT_SUCCESS;
}

string getConfName(std::string strProcessName)
{
	size_t found = strProcessName.find_last_of("/\\");
	return (strProcessName.substr(++found) + ".conf");
}

void runService(int nKey)
{
	int nInit = TRUE;
	int nTmp = -1;
	int nMsgID = -1;
	extern char *__progname;

	CController *controller = CController::getInstance();
	CConfig *config = new CConfig();
	string *pstrConf = new string(getConfName(__progname));
	_log("Get Config File : %s", pstrConf->c_str());
	if(FALSE != config->loadConfig(*pstrConf))
	{
		_setLogPath(config->getValue("LOG", "log").c_str());
		convertFromString(nMsgID, config->getValue("MSQ", "id"));
		if(controller->initMessage(nMsgID))
		{
			if(0 == config->getValue("SERVER AMX", "enable").compare("yes"))
			{
				convertFromString(nTmp, config->getValue("SERVER AMX", "port"));
				if(!controller->startServerAMX(config->getValue("SERVER AMX", "ip"), nTmp, nMsgID))
				{
					nInit = FALSE;
					_log("[Controller] Create Server AMX Service Fail. Port : %d , Message ID : %d", nTmp, nMsgID);
				}
				else
				{
					_log("[Controller] Create Server AMX Service Success. Port : %d , Message ID : %d", nTmp, nMsgID);
				}
			}

			if(0 == config->getValue("SERVER DEVICE", "enable").compare("yes"))
			{
				convertFromString(nTmp, config->getValue("SERVER DEVICE", "port"));
				if(!controller->startServerDevice(config->getValue("SERVER DEVICE", "ip"), nTmp, nMsgID))
				{
					nInit = FALSE;
					_log("[Controller] Create Server DEVICE Service Fail. Port : %d , Message ID : %d", nTmp, nMsgID);
				}
				else
				{
					_log("[Controller] Create Server DEVICE Service Success. Port : %d , Message ID : %d", nTmp,
							nMsgID);
					string strTimer = config->getValue("TIMER", "amx_busy");
					if(!strTimer.empty())
					{
						convertFromString(nTmp, strTimer);
						controller->setAMXBusyTimer(nTmp);
					}
				}
			}
		}
		else
		{
			nInit = FALSE;
			_log("[Controller] Create Message Queue Fail");
		}
	}
	else
	{
		nInit = FALSE;
		_log("[Controller] Load Configuration File Fail");
	}
	delete pstrConf;
	delete config;

	if(TRUE == nInit)
	{
		cout << "\n<============= (◕‿‿◕｡) ... Service Start Run ... ԅ(¯﹃¯ԅ) =============>\n" << endl;
		controller->run(EVENT_FILTER_CONTROLLER, "Controller");
		CMessageHandler::closeMsg(CMessageHandler::registerMsq(nMsgID));
		cout << "\n<============= ( #｀Д´) ... Service Stop Run ... (╬ ಠ 益ಠ) =============>\n" << endl;
		controller->stopServer();
	}
	delete controller;
}
/**
 * process options
 */
void options(int argc, char **argv)
{
	int c;

	while((c = getopt(argc, argv, "M:P:F:m:p:f:H:h")) != -1)
	{
		switch(c)
		{
		case 'H':
		case 'h':
			printf("this is help\n");
			break;
		}
	}
}

