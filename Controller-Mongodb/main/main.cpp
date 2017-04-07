/*
 * main.cpp
 *
 *  Created on: 2016年5月9日
 *      Author: Jugo
 */

#include <unistd.h>
#include <string.h>
#include <iostream>

#include "CProcessHandler.h"
#include "CMessageHandler.h"
#include "event.h"
#include "LogHandler.h"
#include "CConfig.h"
#include "utility.h"
#include "common.h"
#include "CController.h"

using namespace std;

string getConfName(string strProcessName);
void options(int argc, char **argv);
static void runService();

int main(int argc, char* argv[])
{
	extern char *__progname;
	openlog(__progname, LOG_PID, LOG_LOCAL0);

	// Run Process
	CProcessHandler::runProcess(runService);

	closelog();
	return EXIT_SUCCESS;
}

string getConfName(string strProcessName)
{
	size_t found = strProcessName.find_last_of("/\\");
	return (strProcessName.substr(++found) + ".conf");
}

void runService()
{
	int nInit = TRUE;
	int nTmp = -1;
	extern char *__progname;

	CController *controller = CController::getInstance();
	CConfig *config = new CConfig();
	string *pstrConf = new string(getConfName(__progname));
	printf("Get Config File : %s", pstrConf->c_str());
	if(FALSE != config->loadConfig(*pstrConf))
	{
		_setLogPath(config->getValue("LOG", "log").c_str());
		if(controller->initMessage(EVENT_MSQ_KEY_CONTROLLER_MONGODB))
		{
			convertFromString(nTmp, config->getValue("SERVER TRACKER", "port"));
			if(!controller->startTrackerServer(0, nTmp))
			{
				nInit = FALSE;
				_log("[Controller] Start Tracker Server Fail");
			}
			if(!controller->startMongoClient())
			{
				nInit = FALSE;
				_log("[Controller] Start Mongo Client Fail");
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
		_log("\n<============= (◕‿‿◕｡) ... Service Start Run ... ԅ(¯﹃¯ԅ) =============>\n");
		controller->run(EVENT_FILTER_CONTROLLER, "Controller");
		controller->stop();
		CMessageHandler::release();
	}
	_close(); // close log file
	delete controller;
	_log("\n<============= ( #｀Д´) ... Service Stop Run ... (╬ ಠ 益ಠ) =============>\n");
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

