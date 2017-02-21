/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include "common.h"
#include "CController.h"
#include "CTimer.h"
#include "CTransferTracker.h"
#include "CTransferUser.h"
#include "LogHandler.h"
#include "config.h"
#include <map>
#include <string>

using namespace std;

static CController * controller = 0;

void onTimer(int nId)
{
	if (controller)
	{
		controller->OnTimer(nId);
	}
}

void CController::OnTimer(int nId)
{
	if (mnBusy)
	{
		printf("[Controller] System Busy, Ignore Sync.");
		return;
	}

	mnBusy = TRUE;

	transUser->start();
	transTracker->start();

	mnBusy = FALSE;
}

CController::CController() :
		CObject(), mnBusy(FALSE), transUser(new CTransferUser), transTracker(new CTransferTracker)
{

}

CController::~CController()
{
	delete transUser;
	delete transTracker;
}

CController* CController::getInstance()
{
	if (0 == controller)
	{
		controller = new CController();
	}
	return controller;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{

}

int CController::start()
{
#ifdef SYNCALL_USER
	_log("[Controller] Run SYNCALL_USER");
	transUser->start();
#else
#ifdef SYNCALL_TRACKER
	_log("[Controller] Run SYNCALL_TRACKER");
	transTracker->start();
#else
	SetTimer(666, 3, TIMER_DU, onTimer);
#endif
#endif

	return TRUE;
}

int CController::stop()
{
	KillTimer(666);
	return FALSE;
}

void CController::setPsql(const char *szHost, const char *szPort, const char *szDB, const char *szUser,
		const char *szPassword)
{
	extern map<string, string> mapPsqlSetting;
	mapPsqlSetting["host"] = szHost;
	mapPsqlSetting["port"] = szPort;
	mapPsqlSetting["database"] = szDB;
	mapPsqlSetting["user"] = szUser;
	mapPsqlSetting["password"] = szPassword;
}

void CController::setMysql(const char *szHost, const char *szPort, const char *szDB, const char *szUser,
		const char *szPassword)
{
	extern map<string, string> mapMysqlSetting;
	mapMysqlSetting["host"] = szHost;
	mapMysqlSetting["port"] = szPort;
	mapMysqlSetting["database"] = szDB;
	mapMysqlSetting["user"] = szUser;
	mapMysqlSetting["password"] = szPassword;
}
