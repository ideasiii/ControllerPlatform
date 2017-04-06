/*
 * CController.cpp
 *
 *  Created on: 2017年03月14日
 *      Author: Jugo
 */

#include <set>
#include <map>
#include "common.h"
#include "LogHandler.h"
#include "CController.h"
#include "CMysqlHandler.h"
#include "CMongoDBHandler.h"
#include "JSONObject.h"
#include "utility.h"

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

void CController::setMongoDB(const char *szHost, const char *szPort)
{
	extern map<string, string> mapMongodb;
	mapMongodb["host"] = szHost;
	mapMongodb["port"] = szPort;
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

	if(syncColume("tracker_poya_android", "1472188038304"))
	{
		syncData("tracker_poya_android", "1472188038304");
	}

	if(syncColume("tracker_poya_ios", "1472188091474"))
	{
		syncData("tracker_poya_ios", "1472188091474");
	}

}

int CController::getDestFields(std::string strTableName, std::set<std::string> &sFields)
{
	extern map<string, string> mapMysqlDestination;
	int nRet;

	nRet = mysql->connect(mapMysqlDestination["host"], mapMysqlDestination["database"], mapMysqlDestination["user"],
			mapMysqlDestination["password"]);
	if(TRUE == nRet)
	{
		mysql->getFields(strTableName, sFields);
		mysql->close();
		nRet = TRUE;
	}

	return nRet;
}

int CController::syncColume(std::string strTable, std::string strAppId)
{
	set<string> sFields;
	int nRet;
	extern map<string, string> mapMysqlSource;
	extern map<string, string> mapMysqlDestination;
	list<map<string, string> > listRest;
	string strSQL;

	if(!getDestFields(strTable, sFields))
	{
		_log("[CController] syncColume fail");
		return FALSE;
	}

	nRet = mysql->connect(mapMysqlSource["host"], "field", mapMysqlSource["user"], mapMysqlSource["password"]);
	if(FALSE == nRet)
	{
		_log("[CController] Mysql Error: %s", mysql->getLastError().c_str());
		return FALSE;
	}

	nRet = mysql->query("SELECT field FROM device_field WHERE id = '" + strAppId + "'", listRest);
	mysql->close();

	if(TRUE == nRet)
	{
		string strValue;
		for(list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i)
		{
			for(map<string, string>::iterator j = i->begin(); j != i->end(); ++j)
			{
				strValue = j->second;
				std::transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
				if(sFields.find(trim(strValue)) == sFields.end())
				{
					strSQL = format("ALTER TABLE %s ADD COLUMN %s TEXT;", strTable.c_str(), trim(strValue).c_str());
					nRet = mysql->connect(mapMysqlDestination["host"], mapMysqlDestination["database"],
							mapMysqlDestination["user"], mapMysqlDestination["password"]);
					if(FALSE == nRet)
					{
						_log("[CController] Mysql Error: %s", mysql->getLastError().c_str());
						return FALSE;
					}
					if(FALSE == mysql->sqlExec(strSQL))
					{
						if(1062 != mysql->getLastErrorNo())
							_log("[CController] Mysql sqlExec Error: %s", mysql->getLastError().c_str());
					}
					mysql->close();
				}
			}
		}
	}

	return TRUE;
}

int CController::syncData(string strTable, string strAppId)
{
	list<string> listJSON;
	extern map<string, string> mapMongodb;
	mongo->connectDB(mapMongodb["host"], mapMongodb["port"]);
	BSONObj query = BSON(
			"create_date" << BSON("$gte" << getMysqlLastDate(strTable) ) << "ID" << BSON("$regex" << strAppId));
	mongo->query("access", "mobile", query, listJSON);
	mongo->close();
	return TRUE;
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
		else
		{
			_log("[CController] getMysqlLastDate Mysql Error: %s", mysql->getLastError().c_str());
		}
		mysql->close();
	}

	return strRet;
}

