/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include <list>
#include <ctime>


#include "CSocketServer.h"
#include "CSocketClient.h"
#include "event.h"
#include "utility.h"
#include "CCmpHandler.h"
#include "CController.h"
#include "CDataHandler.cpp"
#include "CSqliteHandler.h"
#include "CThreadHandler.h"
#include "JSONObject.h"
#include "JSONArray.h"
#include "ICallback.h"
#include "packet.h"

using namespace std;

static CController * controller = 0;

/**
 * Define Socket Client ReceiveFunction
 */
int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
	//controlcenter->receiveCMP(nSocketFD, nDataLen, pData);
	return 0;
}

/**
 *  Define Socket Server Receive Function
 */
int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
{
	//controlcenter->receiveClientCMP(nSocketFD, nDataLen, pData);
	return 0;
}



CController::CController() :
		CObject(), cmpParser(CCmpHandler::getInstance()), sqlite(0), tdEnquireLink(new CThreadHandler)
{

}

CController::~CController()
{
	delete cmpParser;
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
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_RECEIVE:

		break;
	default:
		_log("[Controller] Unknow message command: %d", nCommand);
		break;
	}
}

int CController::start(string strDB)
{
	if (strDB.empty())
		return FALSE;

	sqlite = new CSqliteHandler();
	if (0 == sqlite)
		return FALSE;

	string strSQL =
			"CREATE TABLE IF NOT EXISTS cpu(pre_cpu_time TEXT , cur_cpu_time TEXT, total_delta_time TEXT, total_cpu_idle TEXT, cpu_usage TEXT,`create_time`	date DEFAULT (datetime('now','localtime')) );";
	;
	list<string> listTable;
	listTable.push_back(strSQL);
	if (!sqlite->connectDB(strDB, listTable))
		return FALSE;

	// test
	for (int i = 0; i < 100; ++i)
	{
		strSQL =
				format(
						"INSERT INTO cpu(pre_cpu_time , cur_cpu_time , total_delta_time , total_cpu_idle , cpu_usage ) VALUES('%d','%d','%d','%d','%d')",
						100 + i, 200 + i, 300 + i, 400 + i, 500 + i);
		sqlite->sqlExec(strSQL);
	}

	JSONArray jsonArray;
	sqlite->query("SELECT * FROM cpu", jsonArray);
	_log("[Controller] monitor data: %s", jsonArray.toString().c_str());

	return TRUE;
}

int CController::stop()
{
	return FALSE;
}
