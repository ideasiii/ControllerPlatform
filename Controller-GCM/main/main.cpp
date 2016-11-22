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
#include "CSqliteHandler.h"

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
	string strConf;
	extern char *__progname;
	getConfName(__progname);

	LogHandler *logAgent = LogHandler::getInstance();
	CController *controller = CController::getInstance();
	CConfig *config = new CConfig();
	string *pstrConf = new string(getConfName(__progname));
	_log("Get Config File : %s", pstrConf->c_str());
	if (FALSE != config->loadConfig(*pstrConf))
	{
		logAgent->setLogPath(config->getValue("LOG", "log"));
		convertFromString(nMsgID, config->getValue("MSQ", "id"));
		if (controller->initMessage(nMsgID))
		{
			if (0 == config->getValue("SERVER", "enable").compare("yes"))
			{
				convertFromString(nTmp, config->getValue("SERVER", "port"));
				if (!controller->startServer(nTmp, nMsgID))
				{
					nInit = FALSE;
				}
				else
				{
					_log("[Controller] Create Server Service Success. Port : %d", nTmp);
				}
			}

			if (0 == config->getValue("CENTER", "enable").compare("yes"))
			{
				convertFromString(nTmp, config->getValue("CENTER", "port"));
			}

			strConf = config->getValue("SQLITE", "db_controller");
			if (!strConf.empty())
			{
				if (!controller->startSqlite(DB_CONTROLLER, strConf))
				{
					nInit = FALSE;
				}
			}
		}
		else
		{
			nInit = FALSE;
		}
	}
	else
	{
		nInit = FALSE;
	}
	delete pstrConf;
	delete config;

	if (TRUE == nInit)
	{
		cout << "\n<============= (◕‿‿◕｡) ... Service Start Run ... p(^-^q) =============>\n" << endl;
		controller->run(EVENT_FILTER_CONTROLLER);
		CMessageHandler::closeMsg(CMessageHandler::registerMsq(nMsgID));
		cout << "\n<============= ( #｀Д´) ... Service Stop Run ... (╬ ಠ 益ಠ) =============>\n" << endl;
		controller->stopServer();
	}
	delete controller;
}

/**
 *Controller Service Run

 void runService(int argc, char* argv[])
 {
 std::string strArgv;
 std::string strConf;
 std::string strSqliteDBController;
 std::string strSqliteDBIdeas;
 int nServerPort = 6607;

 LogHandler *logAgent = LogHandler::getInstance();
 logAgent->setLogPath("/data/opt/tomcat/webapps/logs/center.log");

 options(argc, argv);

 CControlCenter *controlCenter = CControlCenter::getInstance();

 strArgv = argv[0];

 size_t found = strArgv.find_last_of("/\\");
 std::string strProcessName = strArgv.substr(++found);

 strConf = strProcessName + ".conf";

 if (!strConf.empty())
 {
 CConfig *config = new CConfig();
 if ( FALSE != config->loadConfig(strConf))
 {
 logAgent->setLogPath(config->getValue("LOG", "log"));
 convertFromString(nServerPort, config->getValue("SERVER", "port"));
 strSqliteDBController = config->getValue("SQLITE", "db_controller");
 strSqliteDBIdeas = config->getValue("SQLITE", "db_ideas");
 }
 delete config;
 }

 if (controlCenter->initMessage( MSG_ID) && controlCenter->startServer(nServerPort)
 && controlCenter->startMongo("127.0.0.1", 27027))
 {
 _log("<============= (◕‿‿◕｡) ... Service Start Run ... p(^-^q) =============>");
 controlCenter->run( EVENT_FILTER_CONTROL_CENTER);
 _log("<============= ( #｀Д´) ... Service Stop Run ... (╬ ಠ 益ಠ) =============>");
 controlCenter->stopServer();
 }
 else
 {
 closeMessage();
 PSigHander(SIGINT);
 }

 _log("[Process] Child process say: good bye~");
 delete logAgent;
 }
 */
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

