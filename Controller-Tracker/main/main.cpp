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

volatile int flag = 0;
pid_t child_pid = -1; //Global

int Watching();
void CSigHander(int signo);
void PSigHander(int signo);
void OtherSigHander(int signo);
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

		printf("Father fork child.........\n");
		child_pid = fork();
		printf("After fork child.........PID=%d \n");
		if (child_pid == -1)
		{
			printf("fork child fail.........\n");
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

			signal(SIGQUIT, OtherSigHander);
			signal(SIGILL, OtherSigHander);
			signal(SIGTRAP, OtherSigHander);
			signal(SIGABRT, OtherSigHander);
			signal(SIGIOT, OtherSigHander);
			signal(SIGBUS, OtherSigHander);
			signal(SIGFPE, OtherSigHander);
			signal(SIGKILL, OtherSigHander);
			signal(SIGUSR1, OtherSigHander);
			signal(SIGSEGV, OtherSigHander);
			signal(SIGUSR2, OtherSigHander);
			signal(SIGALRM, OtherSigHander);
			signal(SIGSTKFLT, OtherSigHander);
			//signal(SIGCLD, OtherSigHander);
			//signal(SIGCHLD, OtherSigHander);
			//signal(SIGCONT, OtherSigHander);
			//signal(SIGSTOP, OtherSigHander);
			signal(SIGTSTP, OtherSigHander);
			//signal(SIGTTIN, OtherSigHander);
			//signal(SIGTTOU, OtherSigHander);
			//signal(SIGURG, OtherSigHander);
			//signal(SIGXCPU, OtherSigHander);
			signal(SIGXFSZ, OtherSigHander);
			//signal(SIGVTALRM, OtherSigHander);
			//signal(SIGPROF, OtherSigHander);
			//signal(SIGWINCH, OtherSigHander);
			//signal(SIGIO, OtherSigHander);
			signal(SIGPWR, OtherSigHander);
			signal(SIGSYS, OtherSigHander);
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

		signal(SIGQUIT, OtherSigHander);
		signal(SIGILL, OtherSigHander);
		signal(SIGTRAP, OtherSigHander);
		signal(SIGABRT, OtherSigHander);
		signal(SIGIOT, OtherSigHander);
		signal(SIGBUS, OtherSigHander);
		signal(SIGFPE, OtherSigHander);
		signal(SIGKILL, OtherSigHander);
		signal(SIGUSR1, OtherSigHander);
		signal(SIGSEGV, OtherSigHander);
		signal(SIGUSR2, OtherSigHander);
		signal(SIGALRM, OtherSigHander);
		signal(SIGSTKFLT, OtherSigHander);
		//signal(SIGCLD, OtherSigHander);
		//signal(SIGCHLD, OtherSigHander);
		//signal(SIGCONT, OtherSigHander);
		//signal(SIGSTOP, OtherSigHander);
		signal(SIGTSTP, OtherSigHander);
		//signal(SIGTTIN, OtherSigHander);
		//signal(SIGTTOU, OtherSigHander);
		//signal(SIGURG, OtherSigHander);
		//signal(SIGXCPU, OtherSigHander);
		signal(SIGXFSZ, OtherSigHander);
		//signal(SIGVTALRM, OtherSigHander);
		//signal(SIGPROF, OtherSigHander);
		//signal(SIGWINCH, OtherSigHander);
		//signal(SIGIO, OtherSigHander);
		signal(SIGPWR, OtherSigHander);
		signal(SIGSYS, OtherSigHander);
		syslog( LOG_INFO, "controller child process has been invoked");

		/*
		 signal(SIGQUIT, OtherSigHander);
		 signal(SIGILL, OtherSigHander);
		 signal(SIGTRAP, OtherSigHander);
		 signal(SIGABRT, OtherSigHander);
		 signal(SIGIOT, OtherSigHander);
		 signal(SIGBUS, OtherSigHander);
		 signal(SIGFPE, OtherSigHander);
		 signal(SIGKILL, OtherSigHander);
		 signal(SIGUSR1, OtherSigHander);
		 signal(SIGSEGV, OtherSigHander);
		 signal(SIGUSR2, OtherSigHander);
		 signal(SIGALRM, OtherSigHander);
		 signal(SIGSTKFLT, OtherSigHander);
		 signal(SIGCLD, OtherSigHander);
		 signal(SIGCHLD, OtherSigHander);
		 signal(SIGCONT, OtherSigHander);
		 signal(SIGSTOP, OtherSigHander);
		 signal(SIGTSTP, OtherSigHander);
		 signal(SIGTTIN, OtherSigHander);
		 signal(SIGTTOU, OtherSigHander);
		 signal(SIGURG, OtherSigHander);
		 signal(SIGXCPU, OtherSigHander);
		 signal(SIGXFSZ, OtherSigHander);
		 signal(SIGVTALRM, OtherSigHander);
		 signal(SIGPROF, OtherSigHander);
		 signal(SIGWINCH, OtherSigHander);
		 signal(SIGIO, OtherSigHander);
		 signal(SIGPWR, OtherSigHander);
		 signal(SIGSYS, OtherSigHander);*/

		w = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
		printf("Father jumo .........: %d , status=%d\n", (int) w, status);
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
void OtherSigHander(int signo)
{
	printf("************[Other Signal] Child Received signal %d *************", signo);

	//closeMessage();
	//sleep( 3 );
//	kill( child_pid, SIGKILL );
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

	strArgv = argv[0];

	size_t found = strArgv.find_last_of("/\\");
	std::string strProcessName = strArgv.substr(++found);

	strConf = strProcessName + ".conf";
	_DBG("Config file is:%s", strConf.c_str())
	Controller *controller = Controller::getInstance();

	if (controller->init(strConf) && -1 != controller->initMessage( MSG_ID))
	{
		if (controller->startServer())
		{
			controller->connectCenter();
			controller->connectMongoDB();
			printf("<============= Service Start Run =============>\n");
			controller->run( EVENT_FILTER_CONTROLLER);
			printf("<============= Service Stop Run =============>\n");
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

