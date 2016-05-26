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

#include "CControlCenter.h"
#include "CMessageHandler.h"
#include "common.h"
#include "event.h"
#include "LogHandler.h"
#include "Config.h"
#include "utility.h"

using namespace std;

volatile int flag = 0;
pid_t child_pid = -1; //Global

int Watching();
void CSigHander(int signo);
void PSigHander(int signo);
void closeMessage();
void runService(int argc, char* argv[]);
void options(int argc, char **argv);

int main(int argc, char* argv[])
{

	// parent process run process monitor
	Watching();

	// child process run service
	runService(argc, argv);

	closeMessage();

	return EXIT_SUCCESS;
}

/**
 * Parent watch child status
 */
int Watching()
{
	pid_t w;
	int status;

	openlog("ControlCenter", LOG_PID, LOG_LOCAL0);

	do
	{
		_DBG("[Process] Fork Child Process...");
		child_pid = fork();
		if (child_pid == -1)
		{
			exit( EXIT_FAILURE);
		}

		if (child_pid == 0)
		{
			/**
			 * Child process
			 */
			signal( SIGINT, CSigHander);
			signal( SIGTERM, CSigHander);
			signal( SIGPIPE, SIG_IGN);
			return 0;
		}

		/**
		 * Parent process
		 */
		signal( SIGINT, PSigHander);
		signal( SIGTERM, PSigHander);
		signal( SIGHUP, PSigHander);
		signal( SIGPIPE, SIG_IGN);

		w = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);

		if (w == -1)
		{
			perror("waitpid");
			exit( EXIT_FAILURE);
		}
		if (WIFEXITED(status))
		{
			_DBG("[Process] child exited, status=%d\n", WEXITSTATUS(status));
		}
		else if (WIFSIGNALED(status))
		{
			_DBG("[Process] child killed by signal %d\n", WTERMSIG(status));
		}
		else if (WIFSTOPPED(status))
		{
			_DBG("[Process] child stopped by signal %d\n", WSTOPSIG(status));
		}
		else if (WIFCONTINUED(status))
		{
			_DBG("[Process] continued\n");
		}
		else
		{
			_DBG("[Process] receive signal: %d\n", status);
		}
		sleep(3);
	}
	while ( SIGTERM != WTERMSIG(status) && !flag);

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
	if ( SIGHUP == signo)
		return;
	_DBG("[Signal] Parent Received signal %d", signo);
	flag = 1;
	sleep(3);
	kill(child_pid, SIGKILL);
}

/**
 * clean message queue
 */
void closeMessage()
{
	int nMsqId = CMessageHandler::registerMsq(MSG_ID);
	if (0 < nMsqId)
	{
		CMessageHandler::closeMsg(nMsqId);
	}
}

/**
 *Controller Service Run
 */
void runService(int argc, char* argv[])
{

	LogHandler *logAgent = LogHandler::getInstance();
	logAgent->setLogPath("/data/opt/tomcat/webapps/logs/center.log");

	int nServerPort = 6607;

	options(argc, argv);

	std::string strArgv;
	std::string strConf;

	strArgv = argv[0];

	size_t found = strArgv.find_last_of("/\\");
	std::string strProcessName = strArgv.substr(++found);

	strConf = strProcessName + ".conf";
	_log("Config file is:%s", strConf.c_str());

	if (!strConf.empty())
	{
		Config *config = new Config();
		if ( FALSE != config->loadConfig(strConf))
		{
			logAgent->setLogPath(config->getValue("LOG", "log"));
			convertFromString(nServerPort, config->getValue("SERVER", "port"));
		}
		delete config;
	}

	CControlCenter *controlCenter = CControlCenter::getInstance();

	if (controlCenter->init(strConf) && -1 != controlCenter->initMessage( MSG_ID))
	{
		if (controlCenter->startServer(nServerPort))
		{
			_DBG("<============= Service Start Run =============>");
			controlCenter->run( EVENT_FILTER_CONTROL_CENTER);
			_DBG("<============= Service Stop Run =============>");
			controlCenter->stopServer();
		}
	}

	_log("[Process] child process exit");
	delete logAgent;
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

