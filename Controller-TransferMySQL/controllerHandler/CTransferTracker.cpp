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
#include "Indicator.h"
#include <string.h>
#include <stddef.h>

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
	string strLa;
	string strLo;
	const char delimiters[] = ",";
	char *running;
	char *token;

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

	strSQL = "SELECT * FROM " + strTable + " WHERE create_date >= '" + strLastDate + "'";

	_log("[CTransferTracker] syncData run PSQL: %s", strSQL.c_str());
	list<map<string, string> > listRest;
	ppsql->query(strSQL.c_str(), listRest);
	ppsql->close();

	int nCount = 0;
	int nTotal = listRest.size();
	_log("[CTransferTracker] Query PSQL Table %s Count: %d to Insert.", strTable.c_str(), nTotal);

	for (list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i, ++nCount)
	{
		strSQL = "INSERT INTO " + strTable + " (";
		strValues = "VALUES(";

		for (map<string, string>::iterator j = (*i).begin(); j != (*i).end(); ++j)
		{
			if ((*j).first.empty())
				continue;

			strSQL += (*j).first;
			strValues = strValues + "'" + (*j).second + "'";

			if (0 == (*j).first.compare("location") && (0 < (*j).second.length()))
			{
				running = strdupa((*j).second.c_str());
				if (0 != running)
				{
					token = strsep(&running, delimiters);
					if (0 != token)
						strLa = token;
					else
						strLa = "";

					token = strsep(&running, delimiters);
					if (0 != token)
						strLo = token;
					else
						strLo = "";

					strSQL += ",latitude,longitude";
					strValues = strValues + ",'" + strLa + "','" + strLo + "'";
				}
			}

			if ((*i).end() != ++j)
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

		_load(nCount);

		if (FALSE == pmysql->sqlExec(strSQL))
		{
			// mysql error no 1062 is Duplicate insert
			if (1062 != pmysql->getLastErrorNo())
				_log("[CTransferTracker] Mysql sqlExec Error: %s", pmysql->getLastError().c_str());
		}
		else
		{
			_log("[CTransferTracker] run MYSQL: %s", strSQL.c_str());
		}
	}

	listRest.clear();
	pmysql->close();

	_log("[CTransferTracker] Mysql Table %s Total insert Count: %d", strTable.c_str(), nCount);

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

int CTransferTracker::syncDataAll()
{
	int nCount;
	string strSQL;
	int nRet;
	extern map<string, string> mapPsqlSetting;
	extern map<string, string> mapMysqlSetting;

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

	// 取得目前PSQL目前的筆數
	nCount = getPsqlCount("tracker_poya_ios");
	if (0 < nCount)
	{
		strSQL = "SELECT * FROM tracker_poya_ios sort by create_date LIMIT 10000";
	}

	return TRUE;
}

int CTransferTracker::getPsqlCount(const char* szTableName)
{
	string strRet;
	string strSQL;
	int nRet;
	extern map<string, string> mapPsqlSetting;
	extern map<string, string> mapMysqlSetting;
	int nCount = 0;

	if (!ppsql->open(mapPsqlSetting["host"].c_str(), mapPsqlSetting["port"].c_str(), mapPsqlSetting["database"].c_str(),
			mapPsqlSetting["user"].c_str(), mapPsqlSetting["password"].c_str()))
	{
		_log("[CTransferUser] Error: Postgresql Connect Fail");
	}
	else
	{

		strSQL = "SELECT count(*) as count FROM " + szTableName;
		list<map<string, string> > listRest;
		ppsql->query(strSQL.c_str(), listRest);
		if (0 < listRest.size())
		{
			strRet = (*listRest.begin())["count"];
			nCount = atoi(strRet.c_str());
		}
		listRest.clear();
	}
	_log("[CTransferTracker] get table: %s record count: %d", szTableName, nCount);
	return nCount;
}
