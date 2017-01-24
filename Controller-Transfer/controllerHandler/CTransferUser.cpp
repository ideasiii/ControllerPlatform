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

#define DB_PATH_IDEAS "/data/sqlite/ideas.db"

CTransferUser::CTransferUser() :
		sqlite(new CSqliteHandler()), psql(new CPsqlHandler())
{

}

CTransferUser::~CTransferUser()
{
	stop();
	delete sqlite;
	delete psql;
}

int CTransferUser::start()
{
	if (!psql->open("175.98.119.121", "5432", "tracker", "tracker", "ideas123!"))
		return FALSE;

	if (!sqlite->connectDB(DB_PATH_IDEAS))
		return FALSE;

	string strSQL = "SELECT * FROM user WHERE created_date >= '" + getPSqlLastDate() + "'";

	JSONArray jsonArray;
	sqlite->query(strSQL, jsonArray);

	_log("[CTransferUser] JSON Item count: %d", jsonArray.size());
	for (int i = 0; i < jsonArray.size(); ++i)
	{
		JSONObject jsonItem(jsonArray.getJsonObject(i));
		strSQL =
				"INSERT INTO tracker_user (id,app_id,mac,os,phone,fb_id,fb_name,fb_email,fb_account,g_account,t_account,created_date)VALUES('"
						+ jsonItem.getString("id") + "','" + jsonItem.getString("app_id") + "','"
						+ jsonItem.getString("mac", "") + "','" + jsonItem.getString("os", "") + "','"
						+ jsonItem.getString("phone", "") + "','" + jsonItem.getString("fb_id", "") + "','"
						+ jsonItem.getString("fb_name", "") + "','" + jsonItem.getString("fb_email", "") + "','"
						+ jsonItem.getString("fb_account", "") + "','" + jsonItem.getString("g_account", "") + "','"
						+ jsonItem.getString("t_account", "") + "','" + jsonItem.getString("created_date") + "')";
		psql->sqlExec(strSQL.c_str());

	}

	sqlite->close();
	psql->close();
	return TRUE;
}
void CTransferUser::stop()
{
	sqlite->close();
}

string CTransferUser::getPSqlLastDate()
{
	string strRet;
	if (psql)
	{
		string strSQL = "select max(created_date) as maxdate from tracker_user";
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

