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
		_log("[Controller] System Busy, Ignore Sync.");
		return;
	}

	mnBusy = TRUE;

	transUser->start();
//	transTracker->start();

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
	SetTimer(666, 3, TIMER_DU, onTimer);
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
	SETTING_DB psqlSeting;
	psqlSeting.strHost = szHost;
	psqlSeting.strPort = szPort;
	psqlSeting.strDatabase = szDB;
	psqlSeting.strUser = szUser;
	psqlSeting.strPassword = szPassword;
}

void CController::setMysql(const char *szHost, const char *szPort, const char *szDB, const char *szUser,
		const char *szPassword)
{
	SETTING_DB mysqlSetting;
	mysqlSetting.strHost = szHost;
	mysqlSetting.strPort = szPort;
	mysqlSetting.strDatabase = szDB;
	mysqlSetting.strUser = szUser;
	mysqlSetting.strPassword = szPassword;
}
