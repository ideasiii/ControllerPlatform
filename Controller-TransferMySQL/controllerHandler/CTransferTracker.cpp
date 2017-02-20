/*
 * CTransferTracker.cpp
 *
 *  Created on: 2017年1月19日
 *      Author: Jugo
 */

#include <set>
#include <list>
#include "common.h"
#include "utility.h"
#include "LogHandler.h"
#include "CTransferTracker.h"
#include "CPsqlHandler.h"
#include "CMysqlHandler.h"
#include "config.h"

using namespace std;

CTransferTracker::CTransferTracker() :
		pmysql(new CMysqlHandler), ppsql(new CPsqlHandler)
{

}

CTransferTracker::~CTransferTracker()
{
	pmysql->close();
	delete pmysql;

	ppsql->close();
	delete ppsql;
}

int CTransferTracker::start()
{
	_log("============== Transfer Tracker Table Start ============");

	if (syncColume("tracker_poya_android", APP_ID_POYA_ANDROID))
	{
		syncData("tracker_poya_android", APP_ID_POYA_ANDROID);
	}
	else
	{
		_log("[CTransferTracker] Sync %s Colume Fail.", "tracker_poya_android");
	}

	if (syncColume("tracker_poya_ios", APP_ID_POYA_IOS))
	{
		syncData("tracker_poya_ios", APP_ID_POYA_IOS);
	}
	else
	{
		_log("[CTransferTracker] Sync %s Colume Fail.", "tracker_poya_ios");
	}

	return TRUE;
}

int CTransferTracker::syncColume(string strTable, string strAppId)
{
	int nRet;
	extern map<string, string> mapPsqlSetting;
	extern map<string, string> mapMysqlSetting;
	set<string> sFieldsPSQL;
	set<string> sFieldsMYSQL;
	string strSQL;

	// Get Fields From PostgreSQL
	if (!ppsql->open(mapPsqlSetting["host"].c_str(), mapPsqlSetting["port"].c_str(), mapPsqlSetting["database"].c_str(),
			mapPsqlSetting["user"].c_str(), mapPsqlSetting["password"].c_str()))
	{
		_log("[CTransferTracker] Error: Postgresql Connect Fail");
		return FALSE;
	}
	ppsql->getFields(strTable, sFieldsPSQL);
	ppsql->close();

	nRet = pmysql->connect(mapMysqlSetting["host"], mapMysqlSetting["database"], mapMysqlSetting["user"],
			mapMysqlSetting["password"]);
	if (FALSE == nRet)
	{
		_log("[CTransferTracker] syncColume Mysql Error: %s", pmysql->getLastError().c_str());
		return FALSE;
	}
	pmysql->getFields(strTable, sFieldsMYSQL);

	// Compare PSQL & MYSQL Data fields.
	for (set<string>::iterator it = sFieldsPSQL.begin(); sFieldsPSQL.end() != it; ++it)
	{
		if (sFieldsMYSQL.end() == sFieldsMYSQL.find(*it))
		{
			strSQL = format("ALTER TABLE %s ADD COLUMN %s TEXT;", strTable.c_str(), (*it).c_str());
			_log("[CTransferTracker] Sync Field: %s", strSQL.c_str());
			nRet = pmysql->sqlExec(strSQL);
		}
	}
	pmysql->close();

	return nRet;
}

int CTransferTracker::syncData(string strTable, string strAppId)
{
	int nRet;
	extern map<string, string> mapPsqlSetting;
	extern map<string, string> mapMysqlSetting;
	string strLastDate;
	string strSQL;
	string strValues;
	string strId;
	map<string, string> mapItem;
	//list<map<string, string> > listRestMysqlId;

	strLastDate = getMysqlLastDate(strTable);
	if (strLastDate.empty())
	{
		_log("[CTransferTracker] get mysql table:%s last create_date fail", strTable.c_str());
		return FALSE;
	}

	if (!ppsql->open(mapPsqlSetting["host"].c_str(), mapPsqlSetting["port"].c_str(), mapPsqlSetting["database"].c_str(),
			mapPsqlSetting["user"].c_str(), mapPsqlSetting["password"].c_str()))
	{
		_log("[CTransferUser] Error: Postgresql Connect Fail");
		return FALSE;
	}

	nRet = pmysql->connect(mapMysqlSetting["host"], mapMysqlSetting["database"], mapMysqlSetting["user"],
			mapMysqlSetting["password"]);
	if (FALSE == nRet)
	{
		_log("[CTransferUser] Mysql Error: %s", pmysql->getLastError().c_str());
		ppsql->close();
		return FALSE;
	}
#ifdef SYNCALL_TRACKER
	strSQL = "SELECT * FROM " + strTable;
#else
	strSQL = "SELECT * FROM " + strTable + " WHERE create_date >= '" + strLastDate + "'";
#endif
	_log("[CTransferTracker] syncData run PSQL: %s", strSQL.c_str());
	list<map<string, string> > listRest;
	ppsql->query(strSQL.c_str(), listRest);
	ppsql->close();

	int nCount = 0;
	for (list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i, ++nCount)
	{
		strSQL = "INSERT INTO " + strTable + " (";
		strValues = "VALUES(";
		mapItem = *i;
		for (map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
		{
			strSQL += (*j).first;
			strValues = strValues + "'" + (*j).second + "'";
			if (mapItem.end() != ++j)
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
			if (0 == (*j).first.compare("_id"))
			{
				strId = (*j).second;
			}
		}
		//	listRestMysqlId.clear();
		//	pmysql->query("SELECT * FROM " + strTable + " WHERE _id = '" + strId + "'", listRestMysqlId);
		//	if (0 < listRestMysqlId.size())
		//		continue;
		strSQL += strValues;

#ifdef SYNCALL_TRACKER
		_log("[CTransferTracker] run MYSQL: %s", strSQL.c_str());
#endif

		if (FALSE == pmysql->sqlExec(strSQL))
		{
			// mysql error no 1062 is Duplicate insert
			if (1062 != pmysql->getLastErrorNo())
				_log("[CTransferTracker] Mysql sqlExec Error: %s", pmysql->getLastError().c_str());
		}
		else
		{
#ifndef SYNCALL_TRACKER
			_log("[CTransferTracker] run MYSQL: %s", strSQL.c_str());
#endif
		}
	}
	pmysql->close();

	return TRUE;
}

string CTransferTracker::getMysqlLastDate(string strTable)
{
	string strRet;
	string strSQL;
	int nRet;

	list<map<string, string> > listRest;
	extern map<string, string> mapMysqlSetting;
	nRet = pmysql->connect(mapMysqlSetting["host"], mapMysqlSetting["database"], mapMysqlSetting["user"],
			mapMysqlSetting["password"]);
	if (FALSE == nRet)
	{
		_log("[CTransferTracker] getMysqlLastDate Mysql Error: %s", pmysql->getLastError().c_str());
	}
	else
	{
		strSQL = "select max(create_date) as maxdate from " + strTable;

		if (TRUE == pmysql->query(strSQL, listRest))
		{
			strRet = DEFAULT_LAST_DATE;
			string strField;
			string strValue;
			map<string, string> mapItem;
			int nCount = 0;
			for (list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i)
			{
				mapItem = *i;
				for (map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
				{
					strRet = (*j).second.c_str();
				}
			}
		}
	}
	pmysql->close();

	return strRet;
}
