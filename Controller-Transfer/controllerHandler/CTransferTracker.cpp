/*
 * CTransferTracker.cpp
 *
 *  Created on: 2017年1月19日
 *      Author: Jugo
 */

#include "common.h"
#include "LogHandler.h"
#include "CTransferTracker.h"
#include "CMongoDBHandler.h"
#include "CPsqlHandler.h"
#include "CSqliteHandler.h"
#include "JSONArray.h"
#include "JSONObject.h"

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

	if (!mongo->connectDB())
	{
		psql->close();
		_log("[CTransferTracker] MongoDB Connect Fail");
		return FALSE;
	}

	if (!syncColume())
	{
		_log("[CTransferTracker] Sync Database Colume Fail.");
		return FALSE;
	}

	psql->close();
	mongo->close();

	return TRUE;
}

string CTransferTracker::getPSqlLastDate()
{
	string strRet;

	return strRet;
}

int CTransferTracker::syncColume()
{
	if (!sqlite->connectDB(DB_PATH_FIELD))
		return FALSE;

	// Get POYA IOS & Android Fields
	if (0 >= sPoyaFieldIos.size())
	{
		psql->getFields("tracker_poya_ios", sPoyaFieldIos);
	}

	if (0 >= sPoyaFieldAndroid.size())
	{
		psql->getFields("tracker_poya_android", sPoyaFieldIos);
	}

	// Get POYA IOS Sqlite Fields
	string strSQL = "select * from device_field where id = '1472188091474'";
	JSONArray jsonArrayIOS;
	sqlite->query(strSQL, jsonArrayIOS);
	for (int i = 0; i < jsonArrayIOS.size(); ++i)
	{
		JSONObject jsonItem(jsonArrayIOS.getJsonObject(i));
		_log("ios: %s ", jsonItem.getString("field").c_str());
	}
	jsonArrayIOS.release();

	// Get POYA Android Sqlite Fields
	strSQL = "select * from device_field where id = '1472188038304'";
	JSONArray jsonArrayAndroid;
	sqlite->query(strSQL, jsonArrayAndroid);
	for (int j = 0; j < jsonArrayAndroid.size(); ++j)
	{
		JSONObject jsonItem(jsonArrayAndroid.getJsonObject(j));
		_log("android: %s ", jsonItem.getString("field").c_str());
	}
	jsonArrayAndroid.release();

	sqlite->close();
	return TRUE;
}
