/*
 * main.cpp
 *
 *  Created on: 2016年5月9日
 *      Author: Jugo
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "CMessageHandler.h"
#include "common.h"
#include "event.h"
#include "Controller.h"
#include "LogHandler.h"
#include "CConfig.h"
#include "utility.h"

#define MSG_ID							27027

volatile int flag = 0;
pid_t child_pid = -1; //Global

int Watching();
void CSigHander(int signo);
void PSigHander(int signo);
void runService(int argc, char* argv[]);
void options(int argc, char **argv);

int main(int argc, char* argv[])
{

	Watching();
	// child process run service

	runService(argc, argv);

	return EXIT_SUCCESS;
}

/**
 * Parent watch child status
 */
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
			signal( SIGINT, CSigHander);
			signal( SIGTERM, CSigHander);
			signal( SIGPIPE, SIG_IGN);
			syslog( LOG_INFO, "controller child process has been invoked");
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
	while( SIGTERM != WTERMSIG(status) && !flag);

	syslog( LOG_INFO, "controller child process has been terminated");
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
	if( SIGHUP == signo)
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
		if( FALSE != config->loadConfig(strConf))
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

	/** Run Log Agent **/
	LogHandler *logAgent = LogHandler::getInstance();
	logAgent->setLogPath(strLogPath);

	/** Run Mongodb Controller **/
	Controller *controller = Controller::getInstance();

	if(-1 != controller->initMessage( MSG_ID) && controller->startServer(nServerPort))
	{
		_DBG("<============= Mongodb Controller Service Start Run =============>");
		controller->run( EVENT_FILTER_CONTROLLER);
		_DBG("<============= Mongodb Controller Service Stop Run =============>");
		controller->stopServer();
	}
	delete logAgent;
	_DBG("[Process] child process exit");
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

