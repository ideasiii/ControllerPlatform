/*
 * main.cpp
 *
 *  Created on: 2015年10月20日
 *      Author: Louis Ju
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "CController.h"
#include "CProcessHandler.h"

#include "CMessageHandler.h"
#include "common.h"
#include "event.h"
#include "LogHandler.h"
#include "utility.h"
#include "CConfig.h"
#include "LogHandler.h"

using namespace std;

string getConfName(string strProcessName);
void options(int argc, char **argv);
void runService();

int main(int argc, char* argv[])
{

	openlog("controller", LOG_PID, LOG_LOCAL0);
//	LogHandler *logAgent = LogHandler::getInstance(); // this is parent process log handler.
//	logAgent->setLogPath("/data/opt/tomcat/webapps/logs/controllerP.log");

// Read Configuration
	string *pstrConf = new string(getConfName(argv[0]));
	delete pstrConf;

	// Run Process
	CProcessHandler *process = new CProcessHandler();
	process->runProcess(runService);

//	if (controller->initMessage( MSG_ID) && controller->startServer(6607))
//	{
//		cout << "<============= (◕‿‿◕｡) ... Service Start Run ... p(^-^q) =============>\n" << endl;
//		process->runProcess(runService);
//		cout << "<============= ( #｀Д´) ... Service Stop Run ... (╬ ಠ 益ಠ) =============>\n" << endl;
//		controller->stopServer();
//	}

	// Clear Message Queue
	CMessageHandler::closeMsg(CMessageHandler::registerMsq(MSG_ID));

	delete process;
	return EXIT_SUCCESS;
}

std::string getConfName(std::string strProcessName)
{
	size_t found = strProcessName.find_last_of("/\\");
	return (strProcessName.substr(++found) + ".conf");
}

void runService()
{
	CController *controller = CController::getInstance();
	if (controller->initMessage( MSG_ID) && controller->startServer(6607))
	{
		cout << "<============= (◕‿‿◕｡) ... Service Start Run ... p(^-^q) =============>\n" << endl;
		controller->run(EVENT_FILTER_CONTROL_CENTER);
		cout << "<============= ( #｀Д´) ... Service Stop Run ... (╬ ಠ 益ಠ) =============>\n" << endl;
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

