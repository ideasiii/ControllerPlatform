/*
 * CTimer.cpp
 *
 *  Created on: 2016年12月19日
 *      Author: Jugo
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "CTimer.h"
#include "LogHandler.h"
#include <map>

using namespace std;

map<int, TimerCBFun> mapCBF;

void handle(union sigval v)
{
	if (mapCBF.find(v.sival_int) != mapCBF.end())
	{
		(*mapCBF[v.sival_int])(v.sival_int);
	}
}

timer_t SetTimer(int nId, int nSec, int nInterSec, TimerCBFun tcbf)
{

	struct sigevent evp;
	struct itimerspec ts;
	timer_t timerid;

	memset(&evp, 0, sizeof(evp));

	evp.sigev_value.sival_ptr = &timerid;
	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = handle;
	evp.sigev_value.sival_int = nId;

	if (-1 == timer_create(CLOCK_REALTIME, &evp, &timerid))
	{
		_log("timer_create fail");
		return 0;
	}

	_log("Set Timer %d Seconds, Id = %d", nSec, nId);
	mapCBF[nId] = tcbf;

	ts.it_interval.tv_sec = nInterSec; // after first start then every time due.
	ts.it_interval.tv_nsec = 0;
	ts.it_value.tv_sec = nSec; // first stat
	ts.it_value.tv_nsec = 0;

	if (-1 == timer_settime(timerid, 0, &ts, NULL))
	{
		if (mapCBF.find(nId) != mapCBF.end())
			mapCBF.erase(nId);
		return 0;
	}

	_log("Set Timer Success, Timer Id = %d", timerid);
	return timerid;
}

void KillTimer(timer_t ntId, int nId)
{
	if (0 >= ntId)
		return;
	timer_delete(ntId);
	if (mapCBF.find(nId) != mapCBF.end())
		mapCBF.erase(nId);
	_log("[Timer] Kill Timer, timer id = %d, id = %d", ntId, nId);
}

