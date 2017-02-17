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

	if (syncColume("tracker_poya_ios", APP_ID_POYA_IOS))
	{
		syncData("tracker_poya_ios", APP_ID_POYA_IOS);
	}
	else
	{
		_log("[CTransferTracker] Sync %s Colume Fail.", "tracker_poya_ios");
	}

	if (syncColume("tracker_poya_android", APP_ID_POYA_ANDROID))
	{
		syncData("tracker_poya_android", APP_ID_POYA_ANDROID);
	}
	else
	{
		_log("[CTransferTracker] Sync %s Colume Fail.", "tracker_poya_android");
	}

	return TRUE;
}

string CTransferTracker::getPSqlLastDate(string strTableName)
{
	string strRet = DEFAULT_LAST_DATE;

	/*	if (!psql.open(PSQL_HOST, PSQL_PORT, PSQL_DB, PSQL_USER, PSQL_PASSWORD))
	 {
	 _log("[CTransferTracker] Error: Postgresql Connect Fail");
	 }
	 else
	 {
	 string strSQL = "select max(create_date) as maxdate from " + strTableName;
	 list<map<string, string> > listRest;
	 psql.query(strSQL.c_str(), listRest);
	 if (0 < listRest.size())
	 {
	 strRet = (*listRest.begin())["maxdate"];
	 }
	 listRest.clear();
	 psql.close();
	 }
	 */
	return strRet;
}

int CTransferTracker::syncColume(string strTable, string strAppId)
{
	extern map<string, string> mapPsqlSetting;
	extern map<string, string> mapMysqlSetting;
	string strValue;
	set<string> sFields;

	// Get Fields From PostgreSQL
	if (!ppsql->open(mapPsqlSetting["host"].c_str(), mapPsqlSetting["port"].c_str(), mapPsqlSetting["database"].c_str(),
			mapPsqlSetting["user"].c_str(), mapPsqlSetting["password"].c_str()))
	{
		_log("[CTransferTracker] Error: Postgresql Connect Fail");
		return FALSE;
	}
	ppsql->getFields(strTable, sFields);
	ppsql->close();

	for (set<string>::iterator it = sFields.begin(); sFields.end() != it; ++it)
	{
		_DBG("%s", (*it).c_str());
	}
	/*	if (!psql.open(PSQL_HOST, PSQL_PORT, PSQL_DB, PSQL_USER, PSQL_PASSWORD))
	 {
	 _log("[CTransferTracker] Error: Postgresql Connect Fail");
	 return FALSE;
	 }
	 else
	 {
	 psql.getFields(strTable, sFields);
	 }
	 */
	// Get Field From Sqlite
	string strSQL = "select * from device_field where id = '" + strAppId + "'";

	return TRUE;
}

int CTransferTracker::syncData(string strTable, string strAppId)
{
	string strSQL;
	string strSQL_INSERT;
	int nCount = 0;
	list<string> listJSON;
	string strValue;
	CPsqlHandler psql;
	/*
	 if (!psql.open(PSQL_HOST, PSQL_PORT, PSQL_DB, PSQL_USER, PSQL_PASSWORD))
	 {
	 _log("[CTransferTracker] Error: Postgresql Connect Fail");

	 return FALSE;
	 }
	 */
	// Get POYA IOS Field From Sqlite
//	strSQL = "select * from device_field where id = '" + strAppId + "'";
//
//	strSQL = "INSERT INTO " + strTable + " (_id,id,create_date,";
//
//	strSQL += ")VALUES( '";
//
//	for (list<string>::iterator i = listJSON.begin(); i != listJSON.end(); ++i)
//	{
//
//	}
//	psql.close();
//	_log("[CTransferTracker] %s insert count: %d", strTable.c_str(), nCount);
	return TRUE;
}
