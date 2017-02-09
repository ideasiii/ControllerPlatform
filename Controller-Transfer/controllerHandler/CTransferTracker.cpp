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
#include "config.h"

using namespace std;
using namespace mongo;

CTransferTracker::CTransferTracker() :
		mongo(CMongoDBHandler::getInstance())
{

}

CTransferTracker::~CTransferTracker()
{

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
	CPsqlHandler psql;
	if (!psql.open(PSQL_HOST, PSQL_PORT, PSQL_DB, PSQL_USER, PSQL_PASSWORD))
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
	return strRet;
}

int CTransferTracker::syncColume(string strTable, string strAppId)
{
	string strValue;
	set<string> sFields;
	CSqliteHandler sqlite;
	CPsqlHandler psql;

	if (!sqlite.connectDB(DB_PATH_FIELD))
	{
		_log("[CTransferTracker] Error: Sqlite Connect Fail: %s", DB_PATH_FIELD);
		return FALSE;
	}

	// Get Fields From PostgreSQL
	if (!psql.open(PSQL_HOST, PSQL_PORT, PSQL_DB, PSQL_USER, PSQL_PASSWORD))
	{
		_log("[CTransferTracker] Error: Postgresql Connect Fail");
		sqlite.close();
		return FALSE;
	}
	else
	{
		psql.getFields(strTable, sFields);
	}

	// Get Field From Sqlite
	string strSQL = "select * from device_field where id = '" + strAppId + "'";

	JSONArray jsonArray;
	sqlite.query(strSQL, jsonArray);

	for (int i = 0; i < jsonArray.size(); ++i)
	{
		JSONObject jsonItem(jsonArray.getJsonObject(i));
		strValue = jsonItem.getString("field");
		std::transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);

		// Sync sqlite & postgresql fields
		if (sFields.find(strValue) == sFields.end())
		{
			strSQL = format("ALTER TABLE %s ADD COLUMN %s TEXT;", strTable.c_str(), strValue.c_str());
			if (!psql.sqlExec(strSQL.c_str()))
			{
				psql.close();
				sqlite.close();
				jsonArray.release();
				return FALSE;
			}
		}
	}
	jsonArray.release();
	sqlite.close();
	psql.close();
	return TRUE;
}

int CTransferTracker::syncData(string strTable, string strAppId)
{
	string strSQL;
	string strSQL_INSERT;
	int nCount = 0;
	list<string> listJSON;
	string strValue;
	CSqliteHandler sqlite;
	CPsqlHandler psql;

	mongo->connectDB(MONGO_HOST, MONGO_PORT);

	BSONObj query = BSON(
			"create_date" << BSON("$gte" << getPSqlLastDate(strTable) ) << "ID" << BSON("$regex" << strAppId));
	mongo->query("access", "mobile", query, listJSON);
	mongo->close();

	if (!sqlite.connectDB(DB_PATH_FIELD))
	{
		_log("[CTransferTracker] Error: Sqlite Connect Fail: %s", DB_PATH_FIELD);
		return FALSE;
	}

	if (!psql.open(PSQL_HOST, PSQL_PORT, PSQL_DB, PSQL_USER, PSQL_PASSWORD))
	{
		_log("[CTransferTracker] Error: Postgresql Connect Fail");
		sqlite.close();
		return FALSE;
	}

	// Get POYA IOS Field From Sqlite
	strSQL = "select * from device_field where id = '" + strAppId + "'";
	JSONArray jsonArray;
	sqlite.query(strSQL, jsonArray);
	sqlite.close();

	if (0 >= jsonArray.size())
	{
		jsonArray.release();
		return FALSE;
	}

	strSQL = "INSERT INTO " + strTable + " (_id,id,create_date,";
	for (int i = 0; i < jsonArray.size(); ++i)
	{
		JSONObject jsonItem(jsonArray.getJsonObject(i));
		strValue = jsonItem.getString("field");
		std::transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
		strSQL += strValue;
		if (jsonArray.size() != (i + 1))
		{
			strSQL += ",";
		}
	}

	strSQL += ")VALUES( '";

	for (list<string>::iterator i = listJSON.begin(); i != listJSON.end(); ++i)
	{
		JSONObject jsonItem(*i);
		JSONObject oid(jsonItem.getJsonObject("_id"));
		strSQL_INSERT = strSQL + oid.getString("$oid") + "','" + jsonItem.getString("ID") + "','"
				+ jsonItem.getString("create_date") + "','";

		for (int i = 0; i < jsonArray.size(); ++i)
		{
			JSONObject jsonField(jsonArray.getJsonObject(i));
			strValue = jsonField.getString("field");
			strSQL_INSERT += jsonItem.getString(strValue, "");

			if (jsonArray.size() != (i + 1))
			{
				strSQL_INSERT += "','";
			}
			else
			{
				strSQL_INSERT += "');";
			}
		}

		jsonItem.release();

		if (psql.sqlExec(strSQL_INSERT.c_str()))
		{
			++nCount;
		}
	}
	jsonArray.release();
	psql.close();
	_log("[CTransferTracker] %s insert count: %d", strTable.c_str(), nCount);
	return TRUE;
}
