/*
 * CTransferUser.cpp
 *
 *  Created on: 2017年1月16日
 *      Author: Jugo
 */

#include "CTransferUser.h"
#include "common.h"
#include "CSqliteHandler.h"
#include "LogHandler.h"
#include "JSONArray.h"
#include "JSONObject.h"
#include "CPsqlHandler.h"
#include "CMongoDBHandler.h"
#include "config.h"

using namespace std;

CTransferUser::CTransferUser()
{

}

CTransferUser::~CTransferUser()
{

}

int CTransferUser::start()
{
	_log("[CTransferUser] Start");
	string strLastDate;
	CPsqlHandler psql;
	CSqliteHandler sqlite;

	strLastDate = getPSqlLastDate();

	if (!psql.open(PSQL_HOST, PSQL_PORT, PSQL_DB, PSQL_USER, PSQL_PASSWORD))
	{
		_log("[CTransferUser] Error: Postgresql Connect Fail");
		return FALSE;
	}

	if (!sqlite.connectDB(DB_PATH_IDEAS))
	{
		_log("[CTransferUser] Error: Sqlite Connect Fail");
		psql.close();
		return FALSE;
	}

	string strSQL = "SELECT * FROM user WHERE create_date >= '" + strLastDate + "'";

	JSONArray jsonArray;
	sqlite.query(strSQL, jsonArray);

	_log("[CTransferUser] JSON Item count: %d", jsonArray.size());
	for (int i = 0; i < jsonArray.size(); ++i)
	{
		JSONObject jsonItem(jsonArray.getJsonObject(i));
		strSQL =
				"INSERT INTO tracker_user (id,app_id,mac,os,phone,fb_id,fb_name,fb_email,fb_account,g_account,t_account,create_date)VALUES('"
						+ jsonItem.getString("id") + "','" + jsonItem.getString("app_id") + "','"
						+ jsonItem.getString("mac", "") + "','" + jsonItem.getString("os", "") + "','"
						+ jsonItem.getString("phone", "") + "','" + jsonItem.getString("fb_id", "") + "','"
						+ jsonItem.getString("fb_name", "") + "','" + jsonItem.getString("fb_email", "") + "','"
						+ jsonItem.getString("fb_account", "") + "','" + jsonItem.getString("g_account", "") + "','"
						+ jsonItem.getString("t_account", "") + "','" + jsonItem.getString("create_date") + "')";
		jsonItem.release();
		psql.sqlExec(strSQL.c_str());

	}
	jsonArray.release();
	psql.close();
	sqlite.close();

	return TRUE;
}

string CTransferUser::getPSqlLastDate()
{
	string strRet = DEFAULT_LAST_DATE;
	CPsqlHandler psql;

	if (!psql.open(PSQL_HOST, PSQL_PORT, PSQL_DB, PSQL_USER, PSQL_PASSWORD))
	{
		_log("[CTransferUser] Error: Postgresql Connect Fail");
	}
	else
	{
		string strSQL = "select max(create_date) as maxdate from tracker_user";
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

