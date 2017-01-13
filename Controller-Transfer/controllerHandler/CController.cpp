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
#include "CSqliteHandler.h"
#include "CThreadHandler.h"
#include "JSONObject.h"
#include "JSONArray.h"
#include "packet.h"
#include "CTimer.h"
#include "CPsqlHandler.h"
#include "CMongoDBHandler.h"

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
	_log("[Controller] Start Monitor , Timer ID: %d", nId);

	sqlite->connectDB("/data/sqlite/ideas.db");
	sqlite->close();
	if (!mongo->connectDB())
	{
		_log("[Controller] MongoDB Connect Fail");
		return;
	}
	mongo->close();
}

CController::CController() :
		CObject(), sqlite(0), psql(0), mongo(0)
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
	sqlite = new CSqliteHandler();
	if (0 == sqlite)
		return FALSE;

	psql = new CPsqlHandler();
	if (!psql->open("175.98.119.121", "5432", "tracker", "tracker", "ideas123!"))
		return FALSE;

	mongo = CMongoDBHandler::getInstance();

	SetTimer(666, 3, 3, onTimer);
	return TRUE;
}

int CController::stop()
{
	psql->close();
	delete psql;
	psql = 0;

	mongo->close();
	delete mongo;

	return FALSE;
}
