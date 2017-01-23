/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include <list>
#include <ctime>

#include "utility.h"
#include "CController.h"
#include "CDataHandler.cpp"
#include "CThreadHandler.h"
#include "JSONObject.h"
#include "JSONArray.h"
#include "CTimer.h"
#include "CTransferTracker.h"
#include "CTransferUser.h"

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
		_log("[Controller] System Busy, Ignore Sync.");
		return;
	}

	mnBusy = TRUE;

	transUser->start();
//	transTracker->start();

	mnBusy = FALSE;
}

CController::CController() :
		CObject(), transTracker(new CTransferTracker()), transUser(new CTransferUser()), mnBusy(FALSE)
{

}

CController::~CController()
{

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
	SetTimer(666, 3, 10, onTimer);
	return TRUE;
}

int CController::stop()
{
	KillTimer(666);
	return FALSE;
}
