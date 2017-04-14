/*
 * main.cpp
 *
 *  Created on: 2016年07月01日
 *      Author: Jugo
 */

#include <unistd.h>
#include <string.h>

#include "CController.h"
#include "CProcessHandler.h"
#include "CMessageHandler.h"
#include "event.h"
#include "LogHandler.h"
#include "CConfig.h"
#include "utility.h"
#include "common.h"

using namespace std;

string getConfName(string strProcessName);
void options(int argc, char **argv);
void runService(int);

int main(int argc, char* argv[])
{
	extern char *__progname;
	openlog(__progname, LOG_PID, LOG_LOCAL0);
	// Run Process
	process(runService, EVENT_MSQ_KEY_CONTROLLER_SYNCDATA);

	closelog();
	return EXIT_SUCCESS;
}

string getConfName(std::string strProcessName)
{
	size_t found = strProcessName.find_last_of("/\\");
	return (strProcessName.substr(++found) + ".conf");
}

void runService(int nMsqKey)
{
	int nInit = TRUE;
	int nTmp = -1;
	extern char *__progname;

	CMessageHandler::closeMsg(CMessageHandler::registerMsq(EVENT_MSQ_KEY_CONTROLLER_SYNCDATA));

	CController *controller = CController::getInstance();
	CConfig *config = new CConfig();
	string *pstrConf = new string(getConfName(__progname));
	_log("Get Config File : %s", pstrConf->c_str());
	if(FALSE != config->loadConfig(*pstrConf))
	{
		_setLogPath(config->getValue("LOG", "log").c_str());
		if(controller->initMessage(EVENT_MSQ_KEY_CONTROLLER_SYNCDATA))
		{
			controller->setMysqlSource(config->getValue("MYSQL SOURCE", "host").c_str(),
					config->getValue("MYSQL SOURCE", "port").c_str(),
					config->getValue("MYSQL SOURCE", "database").c_str(),
					config->getValue("MYSQL SOURCE", "user").c_str(),
					config->getValue("MYSQL SOURCE", "password").c_str());

			controller->setMysqlDestination(config->getValue("MYSQL DESTINATION", "host").c_str(),
					config->getValue("MYSQL DESTINATION", "port").c_str(),
					config->getValue("MYSQL DESTINATION", "database").c_str(),
					config->getValue("MYSQL DESTINATION", "user").c_str(),
					config->getValue("MYSQL DESTINATION", "password").c_str());

			controller->setMongoDB(config->getValue("MONGODB", "host").c_str(),
					config->getValue("MONGODB", "port").c_str());

			if(!controller->start())
			{
				nInit = FALSE;
				_log("[Controller] Start Sync Data Service Fail");
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

