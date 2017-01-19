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

#include "CMongoDBHandler.h"
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

	_log("[Controller] Start Monitor , Timer ID: %d", nId);

	_log("[Controller] sync ideas.db user start");
	transUser->start();
	_log("[Controller] sync ideas.db user finish");

//	if (!mongo->connectDB())
//	{
//		_log("[Controller] MongoDB Connect Fail");
//	}
//	else
//	{
//		mongo->close();
//	}

	mnBusy = FALSE;
}

CController::CController() :
		CObject(), mongo(0), transUser(new CTransferUser()), mnBusy(FALSE)
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
	mongo = CMongoDBHandler::getInstance();

	SetTimer(666, 3, 3, onTimer);
	return TRUE;
}

int CController::stop()
{

	mongo->close();
	delete mongo;

	return FALSE;
}
