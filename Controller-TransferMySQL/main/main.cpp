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

string getConfName(std::string strProcessName)
{
	size_t found = strProcessName.find_last_of("/\\");
	return (strProcessName.substr(++found) + ".conf");
}

void runService()
{
	int nInit = TRUE;
	int nTmp = -1;
	int nMsgID = -1;
	extern char *__progname;

	CController *controller = CController::getInstance();
	CConfig *config = new CConfig();
	string *pstrConf = new string(getConfName(__progname));
	printf("Get Config File : %s\n", pstrConf->c_str());
	if (FALSE != config->loadConfig(*pstrConf))
	{
		_setLogPath(config->getValue("LOG", "log").c_str());
		convertFromString(nMsgID, config->getValue("MSQ", "id"));
		if (controller->initMessage(nMsgID))
		{
			controller->setPsql(config->getValue("PSQL", "host").c_str(), config->getValue("PSQL", "port").c_str(),
					config->getValue("PSQL", "database").c_str(), config->getValue("PSQL", "user").c_str(),
					config->getValue("PSQL", "password").c_str());
			controller->setMysql(config->getValue("MYSQL", "host").c_str(), config->getValue("MYSQL", "port").c_str(),
					config->getValue("MYSQL", "database").c_str(), config->getValue("MYSQL", "user").c_str(),
					config->getValue("MYSQL", "password").c_str());
			if (!controller->start())
			{
				nInit = FALSE;
				_log("[Controller] Start Service Fail");
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

	if (TRUE == nInit)
	{
		_log("<============= (◕‿‿◕｡) ... Service Start Run ... ԅ(¯﹃¯ԅ) =============>\n");
		controller->run(EVENT_FILTER_CONTROLLER, "Controller");
		controller->stop();
		CMessageHandler::closeMsg(CMessageHandler::registerMsq(nMsgID));
		_log("<============= ( #｀Д´) ... Service Stop Run ... (╬ ಠ 益ಠ) =============>\n");
	}
	_close();
	delete controller;
}
/**
 * process options
 */
void options(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, "M:P:F:m:p:f:H:h")) != -1)
	{
		switch (c)
		{
		case 'H':
		case 'h':
			printf("this is help\n");
			break;
		}
	}
}

