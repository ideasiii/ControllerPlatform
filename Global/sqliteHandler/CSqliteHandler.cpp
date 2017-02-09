/*
 * CSqliteHandler.cpp
 *
 *  Created on: 2015年10月27日
 *      Author: Louis Ju
 */

#include <stdio.h>
#include <sqlite3.h>
#include <ctime>
#include <map>
#include "CSqliteHandler.h"
#include "common.h"
#include "LogHandler.h"
#include "JSONObject.h"
#include "JSONArray.h"

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		_DBG("[Sqlite] %s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

CSqliteHandler::CSqliteHandler() :
		database(0)
{

}

CSqliteHandler::~CSqliteHandler()
{
	close();
	sqlite3_shutdown();
}

int CSqliteHandler::connectDB(string strDBPath, list<string> listTable)
{
	// initialize engine
	int nRet;
	if (SQLITE_OK != (nRet = sqlite3_initialize()))
	{
		_log("[Sqlite] Failed to initialize library: %d\n", nRet);
		return FALSE;
	}

	if (0 != database)
	{
		close();
	}

	// open connection to a DB
	if (SQLITE_OK
			!= (nRet = sqlite3_open_v2(strDBPath.c_str(), &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)))
	{
		_log("[Sqlite] Can't open database: %s", sqlite3_errmsg(database));
		return FALSE;
	}

	for (list<string>::iterator i = listTable.begin(); i != listTable.end(); ++i)
	{
		if ( SQLITE_OK != (nRet = sqlExec(*i)))
		{
			_log("[Sqlite] Can't create Table: %s", sqlite3_errmsg(database));
			return FALSE;
		}
	}

	return TRUE;
}

int CSqliteHandler::connectDB(string strDBPath)
{
	// initialize engine
	int nRet;
	if (SQLITE_OK != (nRet = sqlite3_initialize()))
	{
		_log("[Sqlite] Failed to initialize library: %d\n", nRet);
		return FALSE;
	}

	if (0 != database)
	{
		close();
	}

	// open connection to a DB
	if (SQLITE_OK
			!= (nRet = sqlite3_open_v2(strDBPath.c_str(), &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)))
	{
		_log("[Sqlite] Can't open database: %s", sqlite3_errmsg(database));
		return FALSE;
	}

	return TRUE;
}

void CSqliteHandler::close()
{
	sqlite3_close(database);
	database = 0;
}

int CSqliteHandler::sqlExec(string strSQL)
{
	int nRet = TRUE;
	char *zErrMsg = 0;
	_log("[Sqlite] SQL exec: %s", strSQL.c_str());
	sqlite3_exec(database, "PRAGMA synchronous=OFF", callback, 0, &zErrMsg);
	int rc = sqlite3_exec(database, strSQL.c_str(), callback, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		_log("[Sqlite] SQL exec error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		nRet = FALSE;
	}

	return nRet;
}

int CSqliteHandler::query(string strSQL, JSONArray &jsonArray)
{
	int nRet = TRUE;
	sqlite3_stmt * stmt;
	int row = 0;
	int s = -1;
	int nValue = -1;
	const unsigned char * text;

	sqlite3_prepare_v2(database, strSQL.c_str(), -1, &stmt, 0);

	int cols = sqlite3_column_count(stmt);
	string strColumn;
	string strValue;
	map<int, string> mapColumn;
	for (int nColumeIndex = 0; nColumeIndex < cols; ++nColumeIndex)
	{
		mapColumn[nColumeIndex] = sqlite3_column_name(stmt, nColumeIndex);
		//_log("[Sqlite] query, colume: %s", mapColumn[nColumeIndex].c_str());
	}

	while (1)
	{
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW)
		{
			JSONObject jsonItem;
			for (int nColumeIndex = 0; nColumeIndex < cols; ++nColumeIndex)
			{
				strValue = string((const char*) sqlite3_column_text(stmt, nColumeIndex));
				jsonItem.put(mapColumn[nColumeIndex], strValue);
			}
			jsonArray.add(jsonItem);
		}
		else
		{
			if (s != SQLITE_DONE)
			{
				_log("[Sqlite] SQL:%s exec fail", strSQL.c_str());
				nRet = FALSE;
			}
			break;
		}
	}

	sqlite3_finalize(stmt);

	return nRet;
}

int CSqliteHandler::getFields(string strTableName, set<string> &sFields)
{
	sqlite3_stmt * stmt;
	int row = 0;
	int s = -1;
	int nValue = -1;
	const unsigned char * text;

	string strSQL = "SELECT * FROM " + strTableName + "LIMIT 1";

	sqlite3_prepare_v2(database, strSQL.c_str(), -1, &stmt, 0);

	int cols = sqlite3_column_count(stmt);
	string strColumn;
	string strValue;
	map<int, string> mapColumn;
	for (int nColumeIndex = 0; nColumeIndex < cols; ++nColumeIndex)
	{
		mapColumn[nColumeIndex] = sqlite3_column_name(stmt, nColumeIndex);
		//_log("[Sqlite] query, colume: %s", mapColumn[nColumeIndex].c_str());
	}
	return sFields.size();
}
