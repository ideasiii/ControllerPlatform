/*
 * CTransferTracker.cpp
 *
 *  Created on: 2017年1月19日
 *      Author: Jugo
 */

#include <list>
#include "common.h"
#include "utility.h"
#include "LogHandler.h"
#include "CTransferTracker.h"
#include "CMongoDBHandler.h"
#include "CPsqlHandler.h"
#include "CSqliteHandler.h"
#include "JSONArray.h"
#include "JSONObject.h"
//#include "mongo/client/dbclient.h"

#define DB_PATH_FIELD				"/data/sqlite/field.db"
#define APP_ID_POYA_ANDROID		"1472188038304"
#define APP_ID_POYA_IOS			"1472188091474"

CTransferTracker::CTransferTracker() :
		mongo(CMongoDBHandler::getInstance()), sqlite(new CSqliteHandler()), psql(new CPsqlHandler())
{

}

CTransferTracker::~CTransferTracker()
{
	delete mongo;
}

int CTransferTracker::start()
{
	if (!psql->open("175.98.119.121", "5432", "tracker", "tracker", "ideas123!"))
	{
		_log("[CTransferTracker] Postgresql Connect Fail");
		return FALSE;
	}

// 動態欄位同步處理
	if (syncColume())
	{
		syncData();
	}
	else
	{
		_log("[CTransferTracker] Sync Database Colume Fail.");
	}

	psql->close();

	return TRUE;
}

string CTransferTracker::getPSqlLastDate(string strTableName)
{
	string strRet;
	if (psql)
	{
		string strSQL = "select max(created_date) as maxdate from " + strTableName;
		list<map<string, string> > listRest;
		psql->query(strSQL.c_str(), listRest);
		if (0 < listRest.size())
		{
			strRet = (*listRest.begin())["maxdate"];
		}
	}
	if (strRet.empty())
		strRet = "2015-07-27 00:00:00";
	return strRet;
}

int CTransferTracker::syncColume()
{
	string strValue;

	if (!sqlite->connectDB(DB_PATH_FIELD))
		return FALSE;

	// Get POYA IOS & Android Fields From PostgreSQL
	if (0 >= sPoyaFieldIos.size())
	{
		psql->getFields("tracker_poya_ios", sPoyaFieldIos);
	}

	if (0 >= sPoyaFieldAndroid.size())
	{
		psql->getFields("tracker_poya_android", sPoyaFieldAndroid);
	}

	// Get POYA IOS Field From Sqlite
	string strSQL = "select * from device_field where id = '1472188091474'";
	JSONArray jsonArrayIOS;
	sqlite->query(strSQL, jsonArrayIOS);

	for (int i = 0; i < jsonArrayIOS.size(); ++i)
	{
		JSONObject jsonItem(jsonArrayIOS.getJsonObject(i));
		strValue = jsonItem.getString("field");

		// Sync sqlite & postgresql fields
		if (sPoyaFieldIos.find(strValue) == sPoyaFieldIos.end())
		{
			strSQL = format("ALTER TABLE tracker_poya_ios ADD COLUMN %s TEXT;", strValue.c_str());
			if (!psql->sqlExec(strSQL.c_str()))
			{
				sqlite->close();
				return FALSE;
			}
		}
	}
	jsonArrayIOS.release();
	sPoyaFieldIos.clear();
	psql->getFields("tracker_poya_ios", sPoyaFieldIos);

	// Get POYA Android Field From Sqlite
	strSQL = "select * from device_field where id = '1472188038304'";
	JSONArray jsonArrayAndroid;
	sqlite->query(strSQL, jsonArrayAndroid);
	for (int j = 0; j < jsonArrayAndroid.size(); ++j)
	{
		JSONObject jsonItem(jsonArrayAndroid.getJsonObject(j));
		strValue = jsonItem.getString("field");

		// Sync sqlite & postgresql fields
		if (sPoyaFieldAndroid.find(strValue) == sPoyaFieldAndroid.end())
		{
			strSQL = format("ALTER TABLE tracker_poya_android ADD COLUMN %s TEXT;", strValue.c_str());
			if (!psql->sqlExec(strSQL.c_str()))
			{
				sqlite->close();
				return FALSE;
			}
		}
	}

	sPoyaFieldAndroid.clear();
	psql->getFields("tracker_poya_android", sPoyaFieldAndroid);

	sqlite->close();
	return TRUE;
}

int CTransferTracker::syncData()
{
	mongo->connectDB("127.0.0.1", "27017");
	list<string> listJSON;
	//mongo->query("access", "mobile", "create_date", "2016-12-20 10:18:45", listJSON);
	mongo::BSONObj query = BSON(
			"create_date" << BSON("$gte" << "2016-12-20 00:00:00") << "ID" << BSON("$regex" << "1472188091474"));
	//mongo->query("access", "mobile", "create_date", "$gte", "2016-12-20 00:00:00", listJSON);
	mongo->query("access", "mobile", query, listJSON);
	mongo->close();
//return TRUE;
	string strJSON;
	int nCount = 0;
	for (list<string>::iterator i = listJSON.begin(); i != listJSON.end(); ++i)
	{
		strJSON = *i;
		++nCount;
		cout << strJSON << endl;
	}
	_log("[CTransferTracker] POYA IOS count: %d", nCount);
	return TRUE;
}
