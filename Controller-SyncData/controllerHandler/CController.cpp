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
#include <string.h>
#include <stddef.h>
#include "Indicator.h"

using namespace std;
using namespace mongo;

static CController * controller = 0;

#define APP_ID_READY		1489783587343 // 首席測速APP

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
		//_log("[Controller] System Busy, Ignore Sync.");
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

	if(syncColume("tracker_speed_limit", "1489783587343"))
	{
		syncData("tracker_speed_limit", "1489783587343");
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

int CController::getFields(std::string strAppId, std::set<std::string> &sFields)
{
	string strError;
	int nRet;
	extern map<string, string> mapMysqlSource;
	list<map<string, string> > listRest;

	nRet = mysql->connect(mapMysqlSource["host"], "field", mapMysqlSource["user"], mapMysqlSource["password"]);
	if(FALSE == nRet)
	{
		_log("[CController] Mysql Error: %s", mysql->getLastError().c_str());
		return FALSE;
	}

	nRet = mysql->query("SELECT field FROM device_field WHERE id = '" + strAppId + "'", listRest);
	strError = mysql->getLastError();
	mysql->close();

	if(TRUE == nRet)
	{
		for(list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i)
		{
			for(map<string, string>::iterator j = i->begin(); j != i->end(); ++j)
			{
				sFields.insert(j->second);
			}
		}
	}
	else
	{
		_log("[CController] getFields Mysql Error: %s", strError.c_str());
	}

	return nRet;
}

int CController::syncColume(std::string strTable, std::string strAppId)
{
	set<string> sFields;
	set<string> sFieldsAppId;
	int nRet;
	extern map<string, string> mapMysqlSource;
	extern map<string, string> mapMysqlDestination;
	list<map<string, string> > listRest;
	string strSQL, strValue;

	if(!getDestFields(strTable, sFields))
	{
		_log("[CController] syncColume getDestFields fail");
		return FALSE;
	}

	if(!getFields(strAppId, sFieldsAppId))
	{
		_log("[CController] syncColume getFields fail");
		return FALSE;
	}

	for(set<string>::iterator it = sFieldsAppId.begin(); sFieldsAppId.end() != it; ++it)
	{
		strValue = (*it);
		std::transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
		if(sFields.find(trim(strValue)) == sFields.end())
		{
			strSQL = format("ALTER TABLE %s ADD COLUMN %s TEXT;", strTable.c_str(), trim(strValue).c_str());
			nRet = mysql->connect(mapMysqlDestination["host"], mapMysqlDestination["database"],
					mapMysqlDestination["user"], mapMysqlDestination["password"]);
			if(FALSE == nRet)
			{
				_log("[CController] syncColume Mysql Error: %s", mysql->getLastError().c_str());
				return FALSE;
			}
			if(FALSE == mysql->sqlExec(strSQL))
			{
				if(1062 != mysql->getLastErrorNo())
					_log("[CController] syncColume Mysql sqlExec Error: %s", mysql->getLastError().c_str());
			}
			mysql->close();
		}
	}

	return TRUE;
}

int CController::syncData(string strTable, string strAppId)
{
	int nRet, nCount;
	string strSQL, strValue, strSQL_INSERT, strLa, strLo, strLocation;
	set<string> sFields;
	list<string> listJSON;
	extern map<string, string> mapMongodb;
	extern map<string, string> mapMysqlDestination;
	const char delimiters[] = ",";
	char *running;
	char *token;

	if(!getFields(strAppId, sFields))
	{
		_log("[CController] getDestFields fail");
		return FALSE;
	}

	mongo->connectDB(mapMongodb["host"], mapMongodb["port"]);
	BSONObj query = BSON(
			"create_date" << BSON("$gte" << getMysqlLastDate(strTable.c_str())) << "ID" << BSON("$regex" << strAppId));
	mongo->query("access", "mobile", query, listJSON);
	mongo->close();

	if(0 >= listJSON.size())
		return TRUE;

	nRet = mysql->connect(mapMysqlDestination["host"], mapMysqlDestination["database"], mapMysqlDestination["user"],
			mapMysqlDestination["password"]);

	if(FALSE == nRet)
	{
		_log("[CController] syncData Mysql Error: %s", mysql->getLastError().c_str());
		return FALSE;
	}

	strSQL = "INSERT INTO " + strTable + " (_id,create_date,latitude,longitude,";

	for(set<string>::iterator it = sFields.begin(); sFields.end() != it; ++it)
	{
		strValue = (*it);
		std::transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
		strSQL += strValue;
		if(sFields.end() != ++it)
		{
			strSQL += ",";
		}
		--it;
	}

	strSQL += ")VALUES( '";

	nCount = 0;
	for(list<string>::iterator itJson = listJSON.begin(); listJSON.end() != itJson; ++itJson, ++nCount)
	{
		JSONObject jsonItem(*itJson);
		JSONObject oid(jsonItem.getJsonObject("_id"));
		strSQL_INSERT = strSQL + oid.getString("$oid") + "','" + jsonItem.getString("create_date") + "','";
		strLocation = jsonItem.getString("location");
		if(!strLocation.empty() && 0 < strLocation.length())
		{
			running = strdupa(strLocation.c_str());
			if(0 != running)
			{
				token = strsep(&running, delimiters);
				if(0 != token)
					strLa = token;
				else
					strLa = "";

				token = strsep(&running, delimiters);
				if(0 != token)
					strLo = token;
				else
					strLo = "";

				strSQL_INSERT = strSQL_INSERT + strLa + "','" + strLo + "','";
			}
			else
				strSQL_INSERT += "','','";
		}
		else
		{
			strSQL_INSERT += "','','";
		}

		for(set<string>::iterator itfield = sFields.begin(); sFields.end() != itfield; ++itfield)
		{
			strSQL_INSERT += jsonItem.getString(*itfield, "");

			if(sFields.end() != ++itfield)
			{
				strSQL_INSERT += "','";
			}
			else
			{
				strSQL_INSERT += "');";
			}
			--itfield;
		}

		jsonItem.release();

		//_DBG("%s", strSQL_INSERT.c_str());

		if(FALSE == mysql->sqlExec(strSQL_INSERT))
		{
			if(1062 != mysql->getLastErrorNo())
				_log("[CController] syncData Mysql sqlExec Error: %s", mysql->getLastError().c_str());
		}
		else
		{
			_load(nCount);
		}

	}
	mysql->close();
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

