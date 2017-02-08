/*
 * main.cpp
 *
 *  Created on: 2015年10月19日
 *      Author: Louis Ju
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
	printf("main run-------------------------->\n");
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

			printf("controller child process has been invoked\n");
			return 0;
		}

		/**
		 * Parent process
		 */
		signal( SIGINT, PSigHander);
		signal( SIGTERM, PSigHander);
		signal( SIGHUP, PSigHander);
		signal( SIGPIPE, SIG_IGN);


		syslog( LOG_INFO, "controller child process has been invoked");

		w = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
		//closeMessage();
		//debug use
		//return 0;
		if (w == -1)
		{
			perror("waitpid");
			exit( EXIT_FAILURE);
		}
		if (WIFEXITED(status))
		{
			printf("[Process] child exited, status=%d\n", WEXITSTATUS(status));
		}
		else if (WIFSIGNALED(status))
		{
			printf("[Process] child killed by signal %d\n", WTERMSIG(status));
		}
		else if (WIFSTOPPED(status))
		{
			printf("[Process] child stopped by signal %d\n", WSTOPSIG(status));
		}
		else if (WIFCONTINUED(status))
		{
			printf("[Process] continued\n");
		}
		else
		{
			printf("[Process] child unkown signal %d\n", status);
			sleep(10000);
		}
		sleep(3);
	}
	while ( SIGTERM != WTERMSIG(status) && !flag);

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
	if ( SIGHUP == signo)
		return;
	printf("[Signal] Parent Received signal %d", signo);
	flag = 1;
	closeMessage();
	sleep(3);
	kill(child_pid, SIGKILL);
}


/**
 * clean message queue
 */
void closeMessage()
{
	CMessageHandler *messageHandler = new CMessageHandler;
	messageHandler->init( MSG_ID);
	messageHandler->close();
	delete messageHandler;
}

/**
 *Controller Service Run
 */
void runService(int argc, char* argv[])
{
	options(argc, argv);
	std::string strArgv;
	std::string strConf;
	LogHandler *logAgent = LogHandler::getInstance();
	logAgent->setLogPath("/data/opt/tomcat/webapps/logs/Controller-Tracker.log");
	strArgv = argv[0];

	size_t found = strArgv.find_last_of("/\\");
	std::string strProcessName = strArgv.substr(++found);

	strConf = strProcessName + ".conf";
	_DBG("Config file is:%s", strConf.c_str());
	Controller *controller = Controller::getInstance();

	if (controller->init(strConf) && -1 != controller->initMessage( MSG_ID))
	{
		if (controller->startServer())
		{
			//controller->connectCenter();
			controller->connectMongoDBController();
			_log("<=============Controller-Tracker Service Start Run =============>\n");
			controller->run( EVENT_FILTER_CONTROLLER);
			_log("<=============Controller-Tracker Service Stop Run =============>\n");
			controller->stopServer();
		}
	}
	printf("[Process] child process exit\n");
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

