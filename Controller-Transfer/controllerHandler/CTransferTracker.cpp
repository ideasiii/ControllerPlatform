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
	mongo->close();
	psql->close();
	sqlite->close();
	delete psql;
	delete sqlite;
	delete mongo;
}

int CTransferTracker::start()
{
	if (!psql->open("175.98.119.121", "5432", "tracker", "tracker", "ideas123!"))
	{
		_log("[CTransferTracker] Postgresql Connect Fail");
		return FALSE;
	}

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
		listRest.clear();
	}
	if (strRet.empty())
		strRet = "2015-07-27 00:00:00";
	return strRet;
}

int CTransferTracker::syncColume(string strTable, string strAppId)
{
	string strValue;

	if (!sqlite->connectDB(DB_PATH_FIELD))
		return FALSE;

	// Get Fields From PostgreSQL
	set<string> sFields;
	psql->getFields(strTable, sFields);

	// Get Field From Sqlite
	string strSQL = "select * from device_field where id = '" + strAppId + "'";
	JSONArray jsonArray;
	sqlite->query(strSQL, jsonArray);

	for (int i = 0; i < jsonArray.size(); ++i)
	{
		JSONObject jsonItem(jsonArray.getJsonObject(i));
		strValue = jsonItem.getString("field");
		std::transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);

		// Sync sqlite & postgresql fields
		if (sFields.find(strValue) == sFields.end())
		{
			strSQL = format("ALTER TABLE %s ADD COLUMN %s TEXT;", strTable.c_str(), strValue.c_str());
			if (!psql->sqlExec(strSQL.c_str()))
			{
				sqlite->close();
				jsonArray.release();
				return FALSE;
			}
		}
	}
	jsonArray.release();
	sqlite->close();
	return TRUE;
}

int CTransferTracker::syncData(string strTable, string strAppId)
{
	string strSQL;
	string strSQL_INSERT;
	int nCount = 0;
	list<string> listJSON;
	string strValue;

	mongo->connectDB("127.0.0.1", "27017");

	BSONObj query = BSON(
			"create_date" << BSON("$gte" << getPSqlLastDate(strTable) ) << "ID" << BSON("$regex" << strAppId));
	mongo->query("access", "mobile", query, listJSON);
	mongo->close();

	// Get POYA IOS Field From Sqlite
	if (!sqlite->connectDB(DB_PATH_FIELD))
		return FALSE;
	strSQL = "select * from device_field where id = '" + strAppId + "'";
	JSONArray jsonArrayIOS;
	sqlite->query(strSQL, jsonArrayIOS);
	sqlite->close();

	if (0 >= jsonArrayIOS.size())
	{
		jsonArrayIOS.release();
		return FALSE;
	}

	strSQL = "INSERT INTO " + strTable + " (_id,id,create_date,";
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
		JSONObject jsonItem(*i);
		JSONObject oid(jsonItem.getJsonObject("_id"));
		strSQL_INSERT = strSQL + oid.getString("$oid") + "','" + jsonItem.getString("ID") + "','"
				+ jsonItem.getString("create_date") + "','";

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

		jsonItem.release();
		oid.release();
		//_log(strSQL_INSERT.c_str());
		//break;
		if (psql->sqlExec(strSQL_INSERT.c_str()))
		{
			++nCount;
		}
	}
	jsonArrayIOS.release();
	_log("[CTransferTracker] %s insert count: %d", strTable.c_str(), nCount);
	return TRUE;
}
