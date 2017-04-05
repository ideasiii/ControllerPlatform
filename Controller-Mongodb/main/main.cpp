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

string getConfName(std::string strProcessName)
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
	_log("Get Config File : %s", pstrConf->c_str());
	if(FALSE != config->loadConfig(*pstrConf))
	{
		_setLogPath(config->getValue("LOG", "log").c_str());
		if(controller->initMessage(EVENT_MSQ_KEY_CONTROLLER_MONGODB))
		{
			convertFromString(nTmp, config->getValue("SERVER TRACKER", "port"));
			if(!controller->startTrackerServer(0, nTmp))
			{
				nInit = FALSE;
				_log("[Controller] Start Tracker Service Fail");
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

int Watching()
{
	pid_t w;
	int status;

	openlog("Controller", LOG_PID, LOG_LOCAL0);

	do
	{
		child_pid = fork();
		if(child_pid == -1)
		{
			exit( EXIT_FAILURE);
		}

		if(child_pid == 0)
		{
			/**
			 * Child process
			 */
			signal(SIGINT, CSigHander);
			signal(SIGTERM, CSigHander);
			signal(SIGPIPE, SIG_IGN);
			syslog(LOG_INFO, "controller child process has been invoked");
			return 0;
		}

		/**
		 * Parent process
		 */
		signal(SIGINT, PSigHander);
		signal(SIGTERM, PSigHander);
		signal(SIGHUP, PSigHander);
		signal(SIGPIPE, SIG_IGN);

		w = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);

		if(w == -1)
		{
			perror("waitpid");
			exit( EXIT_FAILURE);
		}
		if(WIFEXITED(status))
		{
			_DBG("[Process] child exited, status=%d\n", WEXITSTATUS(status));
		}
		else if(WIFSIGNALED(status))
		{
			_DBG("[Process] child killed by signal %d\n", WTERMSIG(status));
		}
		else if(WIFSTOPPED(status))
		{
			_DBG("[Process] child stopped by signal %d\n", WSTOPSIG(status));
		}
		else if(WIFCONTINUED(status))
		{
			_DBG("[Process] continued\n");
		}
		sleep(3);
	}
	while(SIGTERM != WTERMSIG(status) && !flag);

	syslog(LOG_INFO, "controller child process has been terminated");
	closelog();
	exit( EXIT_SUCCESS);
	return 1;
}

/**
 * Child signal handler
 */
void CSigHander(int signo)
{
	_DBG("[Signal] Child Received signal %d", signo);
	flag = 1;
}

/**
 * Parent signal handler
 */
void PSigHander(int signo)
{
	if(SIGHUP == signo)
		return;
	_DBG("[Signal] Parent Received signal %d", signo);
	flag = 1;

	sleep(3);
	kill(child_pid, SIGKILL);
}

std::string getConfName(std::string strParam)
{
	std::string strProcessName;
	std::string strArgv;
	std::string strConf;

	strArgv = strParam;
	size_t found = strArgv.find_last_of("/\\");
	strProcessName = strArgv.substr(++found);
	strConf = strProcessName + ".conf";
	_DBG("Config file is:%s", strConf.c_str());

	return strConf;
}

int getPort(std::string strPort)
{
	int nPort = -1;
	if(!strPort.empty())
	{
		convertFromString(nPort, strPort);
	}
	return nPort;
}

/**
 *Controller Service Run
 */
void runService(int argc, char* argv[])
{
	std::string strConf;
	std::string strLogPath = "/data/opt/tomcat/webapps/logs/mongodbController.log";
	int nServerPort = 27027;

	options(argc, argv);

	/** get config file  name **/
	strConf = getConfName(argv[0]);
	if(!strConf.empty())
	{
		CConfig *config = new CConfig();
		if(FALSE != config->loadConfig(strConf))
		{
			strLogPath = config->getValue("LOG", "log");
			nServerPort = getPort(config->getValue("SERVER", "port"));
		}
		else
		{
			_DBG("Load Config File Fail:%s", strConf.c_str());
		}
		delete config;
	}

	_setLogPath(strLogPath.c_str());

	/** Run Mongodb Controller **/
	Controller *controller = Controller::getInstance();

	if(-1 != controller->initMessage(MSG_ID) && controller->startServer(nServerPort))
	{
		_log("\n<============= (◕‿‿◕｡) ... Service Start Run ... ԅ(¯﹃¯ԅ) =============>\n");
		controller->run(EVENT_FILTER_CONTROLLER);
		controller->stopServer();
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

