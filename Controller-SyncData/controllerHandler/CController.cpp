/*
 * CController.cpp
 *
 *  Created on: 2017年03月14日
 *      Author: Jugo
 */

#include <map>
#include "common.h"
#include "LogHandler.h"
#include "CController.h"
#include "CMysqlHandler.h"
#include "CMongoDBHandler.h"
#include "JSONObject.h"

using namespace std;

static CController * controller = 0;

CController::CController() :
		CObject(), mysql(new CMysqlHandler), mongo(CMongoDBHandler::getInstance()), mnBusy(FALSE)
{

}

CController::~CController()
{
	delete mysql;
	delete mongo;
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

void CController::onTimer(int nId)
{
	if(mnBusy)
	{
		_log("[Controller] System Busy, Ignore Sync.");
		return;
	}

	mnBusy = TRUE;

	syncTrackerUser();
	syncTrackerData();

	mnBusy = FALSE;
}

int CController::start()
{
	setTimer(666, 3, 10);
	return TRUE;
}

void CController::stop()
{
	killTimer(666);
}

void CController::setMysqlSource(const char *szHost, const char *szPort, const char *szDB, const char *szUser,
		const char *szPassword)
{
	extern map<string, string> mapMysqlSource;
	mapMysqlSource["host"] = szHost;
	mapMysqlSource["port"] = szPort;
	mapMysqlSource["database"] = szDB;
	mapMysqlSource["user"] = szUser;
	mapMysqlSource["password"] = szPassword;
}

void CController::setMysqlDestination(const char *szHost, const char *szPort, const char *szDB, const char *szUser,
		const char *szPassword)
{
	extern map<string, string> mapMysqlDestination;
	mapMysqlDestination["host"] = szHost;
	mapMysqlDestination["port"] = szPort;
	mapMysqlDestination["database"] = szDB;
	mapMysqlDestination["user"] = szUser;
	mapMysqlDestination["password"] = szPassword;
}

void CController::syncTrackerUser()
{
	extern map<string, string> mapMysqlSource;
	extern map<string, string> mapMysqlDestination;
	list<map<string, string> > listRest;
	string strLastDate;
	string strSQL;
	string strError;
	int nRet;

	// Get Destination Mysql Record last date
	strLastDate = getMysqlLastDate("tracker_user");
	if(strLastDate.empty())
		return;

	strSQL = "SELECT * FROM tracker_user WHERE create_date >= '" + strLastDate + "'";
	nRet = mysql->connect(mapMysqlSource["host"], mapMysqlSource["database"], mapMysqlSource["user"],
			mapMysqlSource["password"]);
	if(FALSE == nRet)
	{
		_log("[CController] Mysql Error: %s", mysql->getLastError().c_str());
		return;
	}

	nRet = mysql->query(strSQL, listRest);
	strError = mysql->getLastError();
	mysql->close();
	if(FALSE == nRet)
	{
		_log("[CController] Mysql Error: %s", strError.c_str());
		return;
	}

	nRet = mysql->connect(mapMysqlDestination["host"], mapMysqlDestination["database"], mapMysqlDestination["user"],
			mapMysqlDestination["password"]);
	if(FALSE == nRet)
	{
		_log("[CController] Mysql Error: %s", mysql->getLastError().c_str());
		return;
	}
	string strField;
	string strValues;
	map<string, string> mapItem;
	int nCount = 0;
	for(list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i)
	{
		strSQL = "INSERT INTO tracker_user (";
		strValues = "VALUES(";
		mapItem = *i;
		for(map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
		{
			strSQL += (*j).first;
			strValues = strValues + "'" + (*j).second + "'";
			if(mapItem.end() != ++j)
			{
				strSQL += ",";
				strValues += ",";
			}
			else
			{
				strSQL += ") ";
				strValues += ")";
			}
			--j;
		}

		strSQL += strValues;

		if(FALSE == mysql->sqlExec(strSQL))
		{
			if(1062 != mysql->getLastErrorNo())
				_log("[CController] Mysql sqlExec Error: %s", mysql->getLastError().c_str());
		}
		else
		{
			_log("[CController] run MYSQL Success: %s", strSQL.c_str());
		}
	}
	mysql->close();

}

void CController::syncTrackerData()
{
	extern map<string, string> mapMysqlDestination;
	list<map<string, string> > listRest;
	string strLastDate;
	string strSQL;
	string strError;
	int nRet;

	// Get Destination Mysql Record last date
	strLastDate = getMysqlLastDate("tracker_poya_android");
	if(strLastDate.empty())
		return;
}

string CController::getMysqlLastDate(const char *szTable)
{
	extern map<string, string> mapMysqlDestination;
	string strRet;
	string strTable;
	int nRet;
	list<map<string, string> > listRest;

	strTable = szTable;
	nRet = mysql->connect(mapMysqlDestination["host"], mapMysqlDestination["database"], mapMysqlDestination["user"],
			mapMysqlDestination["password"]);
	if(FALSE == nRet)
	{
		_log("[CController] getMysqlLastDate Mysql Error: %s", mysql->getLastError().c_str());
	}
	else
	{
		if(TRUE == mysql->query("select max(create_date) as maxdate from " + strTable, listRest))
		{
			strRet = "2015-07-27 00:00:00";
			string strField;
			string strValue;
			map<string, string> mapItem;
			int nCount = 0;
			for(list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i)
			{
				mapItem = *i;
				for(map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
				{
					strRet = (*j).second.c_str();
				}
			}
		}
	}
	mysql->close();

	return strRet;
}

