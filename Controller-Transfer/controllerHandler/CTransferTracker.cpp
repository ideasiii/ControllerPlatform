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

using namespace std;
using namespace mongo;

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
		string strSQL = "select max(create_date) as maxdate from " + strTableName;
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

	// Get POYA IOS Fields From PostgreSQL
	psql->getFields("tracker_poya_ios", sPoyaFieldIos);

	// Get POYA IOS Field From Sqlite
	string strSQL = "select * from device_field where id = '1472188091474'";
	JSONArray jsonArrayIOS;
	sqlite->query(strSQL, jsonArrayIOS);

	for (int i = 0; i < jsonArrayIOS.size(); ++i)
	{
		JSONObject jsonItem(jsonArrayIOS.getJsonObject(i));
		strValue = jsonItem.getString("field");
		std::transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
		_log("[CTransferTracker] ios check field name: %s", strValue.c_str());
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

	// Get POYA Android Fields From PostgreSQL
	psql->getFields("tracker_poya_android", sPoyaFieldAndroid);
	// Get POYA Android Field From Sqlite
	strSQL = "select * from device_field where id = '1472188038304'";
	JSONArray jsonArrayAndroid;
	sqlite->query(strSQL, jsonArrayAndroid);
	for (int j = 0; j < jsonArrayAndroid.size(); ++j)
	{
		JSONObject jsonItem(jsonArrayAndroid.getJsonObject(j));
		strValue = jsonItem.getString("field");
		std::transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
		_log("[CTransferTracker] android check field name: %s", strValue.c_str());
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

	sqlite->close();
	return TRUE;
}

int CTransferTracker::syncData()
{
	string strSQL;
	string strSQL_INSERT;
	int nCount = 0;
	list<string> listJSON;
	string strValue;

	mongo->connectDB("127.0.0.1", "27017");

	BSONObj query =
			BSON(
					"create_date" << BSON("$gte" << getPSqlLastDate("tracker_poya_ios") ) << "ID" << BSON("$regex" << "1472188091474"));
	mongo->query("access", "mobile", query, listJSON);
	mongo->close();

	// Get POYA IOS Field From Sqlite
	if (!sqlite->connectDB(DB_PATH_FIELD))
		return FALSE;
	strSQL = "select * from device_field where id = '1472188091474'";
	JSONArray jsonArrayIOS;
	sqlite->query(strSQL, jsonArrayIOS);
	sqlite->close();

	if (0 >= jsonArrayIOS.size())
		return FALSE;

	strSQL = "INSERT INTO tracker_poya_ios (id,create_date,";
	for (int i = 0; i < jsonArrayIOS.size(); ++i)
	{
		JSONObject jsonItem(jsonArrayIOS.getJsonObject(i));
		strValue = jsonItem.getString("field");
		std::transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
		strSQL += strValue;
		if (jsonArrayIOS.size() != (i + 1))
		{
			strSQL += ",";
		}
	}

	strSQL += ")VALUES( '";

	for (list<string>::iterator i = listJSON.begin(); i != listJSON.end(); ++i)
	{
		++nCount;
		JSONObject jsonItem(*i);

		strSQL_INSERT = strSQL + jsonItem.getString("ID") + "','" + jsonItem.getString("create_date") + "','";

		for (int i = 0; i < jsonArrayIOS.size(); ++i)
		{
			JSONObject jsonField(jsonArrayIOS.getJsonObject(i));
			strValue = jsonField.getString("field");
			strSQL_INSERT += jsonItem.getString(strValue, "");

			if (jsonArrayIOS.size() != (i + 1))
			{
				strSQL_INSERT += "','";
			}
			else
			{
				strSQL_INSERT += "');";
			}
		}

		//_log(strSQL_INSERT.c_str());
		//break;
		psql->sqlExec(strSQL_INSERT.c_str());
	}
	_log("[CTransferTracker] POYA IOS count: %d", nCount);
	return TRUE;
}
