/*
 * CProcessHandler.cpp
 *
 *  Created on: 2016年6月27日
 *      Author: Jugo
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <event.h>

#include "CMessageHandler.h"
#include "CProcessHandler.h"
#include "common.h"
#include "LogHandler.h"

int mnMsqKey;
volatile int flag = 0;
pid_t child_pid = -1; //Global

/**
 * Child signal handler
 */
void CSigHander(int signo)
{
	_log("[Signal] Child Received signal %d", signo);
	flag = 1;
}

/**
 * Parent signal handler
 */
void PSigHander(int signo)
{
	if( SIGHUP == signo)
		return;
	_log("[Signal] Parent Received signal %d", signo);
	flag = 1;
	sleep(3);
	kill(child_pid, SIGKILL);
}

CProcessHandler::CProcessHandler()
{

}

CProcessHandler::~CProcessHandler()
{

}

int CProcessHandler::runProcess(void (*entry)(void))
{
	return process(entry);
}

//========================== Extern Mode ==============================================//

int process(void (*entry)(void))
{
	if(0 == entry)
		return -1;

	pid_t w;
	int status;

	do
	{
		_close();
		child_pid = fork();
		if(-1 == child_pid)
		{
			exit(EXIT_FAILURE);
		}

		if(child_pid == 0)
		{
			/**
			 * Child process
			 */
			signal(SIGINT, CSigHander);
			signal(SIGTERM, CSigHander);
			signal(SIGPIPE, SIG_IGN);

			(*entry)();
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
			exit(EXIT_FAILURE);
		}
		if(WIFEXITED(status))
		{
			_log("[Process] child exited, status=%d\n", WEXITSTATUS(status));
		}
		else if(WIFSIGNALED(status))
		{
			_log("[Process] child killed by signal %d\n", WTERMSIG(status));
		}
		else if(WIFSTOPPED(status))
		{
			_log("[Process] child stopped by signal %d\n", WSTOPSIG(status));
		}
		else if(WIFCONTINUED(status))
		{
			_log("[Process] continued\n");
		}
		else
		{
			_log("[Process] receive signal: %d\n", status);
		}
		sleep(2);
	}
	while(SIGTERM != WTERMSIG(status) && !flag);

	return 1;
}

