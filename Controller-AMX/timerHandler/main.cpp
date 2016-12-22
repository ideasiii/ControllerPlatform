#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "CTimer.h"

using namespace std;

static int scount = 0;
timer_t timerid2;
void onTimer(int nId)
{
	printf("[Timer] thread id = %ld , callback id = %d pid = %ld\n", (long) pthread_self(), nId, (long) getpid());

	if (666 == nId)
		++scount;

	if (3 == scount)
	{
		KillTimer(777);
	}

	if (5 == scount)
	{
		scount = 0;
		timerid2 = SetTimer(777, 2, 1, onTimer);
		printf("[Timer] time2 id = %ld\n", (long) timerid2);
	}
}

int main(int argc, char* argv[])
{
	printf("[Main] process id = %ld\n", (long) getpid());

	timer_t timerid = SetTimer(666, 3, 1, onTimer);
	printf("[Timer] time1 id = %ld\n", (long) timerid);

	timerid2 = SetTimer(777, 2, 1, onTimer);
	printf("[Timer] time2 id = %ld\n", (long) timerid2);

	timerid = SetTimer(888, 3, 0, onTimer);
	printf("[Timer] time3 id = %ld\n", (long) timerid);

	timerid = SetTimer(666, 3, 1, onTimer);
	printf("[Timer] time1 id = %ld\n", (long) timerid);

	pause();
}

