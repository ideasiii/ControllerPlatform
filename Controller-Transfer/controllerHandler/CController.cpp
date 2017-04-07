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

//void onTimer(int nId)
//{
//	if(controller)
//	{
//		controller->OnTimer(nId);
//	}
//}

void CController::onTimer(int nId)
{
	if(mnBusy)
	{
		_log("[Controller] System Busy, Ignore Sync.");
		return;
	}

	mnBusy = TRUE;

	CTransferUser transUser;
	CTransferTracker transTracker;
	transUser.start();
	transTracker.start();

	mnBusy = FALSE;
}

CController::CController() :
		CObject(), mnBusy(FALSE)
{

}

CController::~CController()
{

}

CController* CController::getInstance()
{
	if(0 == controller)
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
	setTimer(666, 3, TIMER_DU);
	return TRUE;
}

int CController::stop()
{
	killTimer(666);
	return FALSE;
}
