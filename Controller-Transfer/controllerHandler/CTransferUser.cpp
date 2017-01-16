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
#define SQL_QUERY_USER "SELECT * FROM user"

CTransferUser::CTransferUser() :
		sqlite(new CSqliteHandler()), psql(new CPsqlHandler())
{

}

CTransferUser::~CTransferUser()
{
	stop();
	delete sqlite;
}

int CTransferUser::start()
{
	if (!psql->open("175.98.119.121", "5432", "tracker", "tracker", "ideas123!"))
		return FALSE;

	if (!sqlite->connectDB(DB_PATH_IDEAS))
		return FALSE;

	JSONArray jsonArray;
	sqlite->query(SQL_QUERY_USER, jsonArray);
	//_log("[CTransferUser] query: %s", jsonArray.toString().c_str());

	for (int i = 0; i < jsonArray.size(); ++i)
	{
		JSONObject jsonItem(jsonArray.getJsonObject(i));
		_log("[CTransferUser] JSON Item : %s", jsonItem.toString().c_str());
	}

	sqlite->close();
	return TRUE;
}
void CTransferUser::stop()
{
	sqlite->close();
}

