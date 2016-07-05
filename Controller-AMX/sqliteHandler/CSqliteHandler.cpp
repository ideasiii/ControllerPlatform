/*
 * CSqliteHandler.cpp
 *
 *  Created on: 2015年10月27日
 *      Author: Louis Ju
 */

#include <stdio.h>
#include <sqlite3.h>
#include <ctime>
#include "CSqliteHandler.h"
#include "common.h"
#include "LogHandler.h"

using namespace std;

static sqlite3 *dbController = 0;
static sqlite3 *dbUser = 0;
static sqlite3 *dbIdeas = 0;				// For MORE SDK
static sqlite3 *dbMdm = 0;				// For MORE MDM

CSqliteHandler* CSqliteHandler::m_instance = 0;

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		_DBG("[Sqlite] %s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

CSqliteHandler::CSqliteHandler()
{
}

CSqliteHandler::~CSqliteHandler()
{
	close();
}

int CSqliteHandler::openControllerDB(const char *dbPath)
{
	int rc = sqlite3_open(dbPath, &dbController);

	int nRet = FALSE;

	if (rc)
	{
		_log("[Sqlite] Can't open controller database: %s", sqlite3_errmsg(dbController));
	}
	else
	{
		sqlExec(dbController, "DROP TABLE controller;");
		const char *sql =
				"CREATE TABLE IF NOT EXISTS controller(id CHAR(50) NOT NULL, status INT NOT NULL, socket_fd INT NOT NULL, created_date DATE, updated_date DATE );";
		if ( SQLITE_OK == sqlExec(dbController, sql))
		{
			nRet = TRUE;
		}
	}

	return nRet;
}
int CSqliteHandler::openUserDB(const char *dbPath)
{
	int rc = sqlite3_open(dbPath, &dbUser);
	int nRet = FALSE;

	if (rc)
	{
		_log("[Sqlite] Can't open user database: %s", sqlite3_errmsg(dbUser));
	}
	else
	{
		const char *sql =
				"CREATE TABLE IF NOT EXISTS user(mac CHAR(20) NOT NULL, account CHAR(20), password CHAR(20), token CHAR(50), created_date DATE );";
		if ( SQLITE_OK == sqlExec(dbUser, sql))
		{
			nRet = TRUE;
		}
	}

	return nRet;
}

int CSqliteHandler::openIdeasDB(const char *dbPath)
{
	int rc = sqlite3_open(dbPath, &dbIdeas);
	int nRet = FALSE;

	if (rc)
	{
		_log("[Sqlite] Can't open ideas database: %s", sqlite3_errmsg(dbIdeas));
	}
	else
	{
		const char *sql =
				"CREATE TABLE IF NOT EXISTS user(id	CHAR(128) NOT NULL, app_id 	CHAR(20)  NOT NULL,mac	CHAR(20), os	CHAR(20), phone	CHAR(20), fb_id CHAR(20), fb_name	CHAR(50), fb_email	CHAR(50), fb_account	CHAR(50), g_account CHAR(50), t_account CHAR(50), created_date DATE DEFAULT (datetime('now','localtime')), PRIMARY KEY(id) );";
		if ( SQLITE_OK == sqlExec(dbIdeas, sql))
		{
			nRet = TRUE;
		}
	}

	return nRet;
}

int CSqliteHandler::openMdmDB(const char *dbPath)
{
	int rc = sqlite3_open(dbPath, &dbMdm);
	int nRet = FALSE;

	if (rc)
	{
		_log("[Sqlite] Can't open mdm database: %s", sqlite3_errmsg(dbMdm));
	}
	else
	{
		const char *sql =
				"CREATE TABLE IF NOT EXISTS `user` (`account`	TEXT NOT NULL,	`password`	TEXT NOT NULL,	`token`	TEXT NOT NULL UNIQUE,	`group`	INTEGER);";
		if ( SQLITE_OK == sqlExec(dbMdm, sql))
		{
			nRet = TRUE;
		}
	}

	return nRet;
}

void CSqliteHandler::close()
{
	sqlite3_close(dbIdeas);
	sqlite3_close(dbController);
	sqlite3_close(dbUser);
	sqlite3_close(dbMdm);

	dbIdeas = 0;
	dbController = 0;
	dbUser = 0;
	dbMdm = 0;
}

CSqliteHandler* CSqliteHandler::getInstance()
{
	if (!m_instance)
	{
		m_instance = new CSqliteHandler();
	}

	return m_instance;
}

int CSqliteHandler::controllerSqlExec(const char *szSql)
{
	int nRet = FAIL;
	nRet = sqlExec(dbController, szSql);
	if ( SQLITE_OK == nRet)
		nRet = SUCCESS;
	return nRet;
}

int CSqliteHandler::ideasSqlExec(const char *szSql)
{
	int nRet = FAIL;
	nRet = sqlExec(dbIdeas, szSql);
	if ( SQLITE_OK == nRet)
		nRet = SUCCESS;
	return nRet;
}

int CSqliteHandler::mdmSqlExec(const char *szSql)
{
	int nRet = FAIL;
	nRet = sqlExec(dbMdm, szSql);
	if ( SQLITE_OK == nRet)
		nRet = SUCCESS;
	return nRet;
}

int CSqliteHandler::sqlExec(sqlite3 *db, const char *szSql)
{
	char *zErrMsg = 0;
	_log("[Sqlite] SQL exec: %s", szSql);
	int rc = sqlite3_exec(db, szSql, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		_log("[Sqlite] SQL exec error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	return rc;
}

int CSqliteHandler::getControllerColumeValue(const char *szTable, const char *szColume,
		std::list<std::string> &listValue)
{
	string strSql = "SELECT " + string(szColume) + " FROM " + string(szTable) + ";";

	sqlite3_stmt * stmt;
	sqlite3_prepare(dbController, strSql.c_str(), strSql.size() + 1, &stmt, NULL); //preparing the statement

	int s;
	int row = 0;

	while (1)
	{
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW)
		{
			const unsigned char * text;
			text = sqlite3_column_text(stmt, 0);
			listValue.push_back(string((const char*) text));
			++row;
		}
		else if (s == SQLITE_DONE)
		{
			break;
		}
		else
		{
			_log("[Sqlite] SQL:%s exec fail", strSql.c_str());
			break;
		}
	}

	sqlite3_finalize(stmt);
	return row;
}

int CSqliteHandler::getControllerColumeValue(const char *szTable, const char *szColume,
		std::list<std::string> &listValue, const char *szValue)
{
	string strSql = "SELECT " + string(szColume) + " FROM " + string(szTable) + " WHERE " + string(szColume) + " = "
			+ string(szValue) + ";";

	sqlite3_stmt * stmt;
	sqlite3_prepare(dbController, strSql.c_str(), strSql.size() + 1, &stmt, NULL); //preparing the statement

	int s;
	int row = 0;

	while (1)
	{
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW)
		{
			const unsigned char * text;
			text = sqlite3_column_text(stmt, 0);
			listValue.push_back(string((const char*) text));
			++row;
		}
		else if (s == SQLITE_DONE)
		{
			break;
		}
		else
		{
			_log("[Sqlite] SQL:%s exec fail", strSql.c_str());
			break;
		}
	}

	sqlite3_finalize(stmt);
	return row;
}

int CSqliteHandler::getControllerColumeValueInt(const char *szSql, std::list<int> &listValue, int nColumeIndex)
{
	sqlite3_stmt * stmt;
	int row = 0;
	int s = -1;
	int nValue = -1;

	sqlite3_prepare(dbController, szSql, strlen(szSql) + 1, &stmt, NULL);

	while (1)
	{
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW)
		{
			nValue = sqlite3_column_int(stmt, nColumeIndex);
			listValue.push_back(nValue);
			++row;
		}
		else
		{
			if (s != SQLITE_DONE)
			{
				_log("[Sqlite] SQL:%s exec fail", szSql);
			}
			break;
		}
	}

	sqlite3_finalize(stmt);

	return row;
}

int CSqliteHandler::ideasSqlExec(const char *szSql, list<string> &listValue, int nColumeIndex)
{
	sqlite3_stmt * stmt;
	int row = 0;
	int s = -1;
	int nValue = -1;
	const unsigned char * text;

	sqlite3_prepare(dbIdeas, szSql, strlen(szSql) + 1, &stmt, NULL);

	while (1)
	{
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW)
		{
			text = sqlite3_column_text(stmt, nColumeIndex);
			listValue.push_back(string((const char*) text));
			++row;
		}
		else
		{
			if (s != SQLITE_DONE)
			{
				_log("[Sqlite] SQL:%s exec fail", szSql);
			}
			break;
		}
	}

	sqlite3_finalize(stmt);

	return row;
}

int CSqliteHandler::mdmSqlExec(const char *szSql, std::list<std::string> &listValue, int nColumeIndex)
{
	return sqlExec(dbMdm, szSql, listValue, nColumeIndex);
}

int CSqliteHandler::sqlExec(sqlite3 *db, const char *szSql, std::list<std::string> &listValue, int nColumeIndex)
{
	sqlite3_stmt * stmt;
	int row = 0;
	int s = -1;
	int nValue = -1;
	const unsigned char * text;

	sqlite3_prepare(db, szSql, strlen(szSql) + 1, &stmt, NULL);

	while (1)
	{
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW)
		{
			text = sqlite3_column_text(stmt, nColumeIndex);
			listValue.push_back(string((const char*) text));
			++row;
		}
		else
		{
			if (s != SQLITE_DONE)
			{
				_log("[Sqlite] SQL:%s exec fail", szSql);
			}
			break;
		}
	}

	sqlite3_finalize(stmt);

	return row;
}

bool CSqliteHandler::isAppIdExist(const std::string strAppId)
{
	bool bExist = false;
	int nState = -1;
	const unsigned char * text;
	string strSql = "select count(*) from app where app_id = '" + strAppId + "';";

	sqlite3_stmt * stmt;
	sqlite3_prepare(dbIdeas, strSql.c_str(), strSql.size() + 1, &stmt, NULL);
	nState = sqlite3_step(stmt);
	switch (nState)
	{
	case SQLITE_ROW:
		text = sqlite3_column_text(stmt, 1);
		_log("[Sqlite] get App Id count: %s", text);
		break;
	case SQLITE_DONE:
		break;
	default:
		_log("[Sqlite] SQL:%s exec fail", strSql.c_str());
		break;
	}
	return bExist;
}
bool CSqliteHandler::getUserAuth(std::string strMAC)
{
	string strSql = "SELECT mac FROM user WHERE mac = '" + strMAC + "';";

	sqlite3_stmt * stmt;
	sqlite3_prepare(dbUser, strSql.c_str(), strSql.size() + 1, &stmt, NULL); //preparing the statement

	int s;
	bool brow = false;

	s = sqlite3_step(stmt);
	if (s == SQLITE_ROW)
	{
		brow = true;
	}
	else if (s == SQLITE_DONE)
	{

	}
	else
	{
		_log("[Sqlite] SQL:%s exec fail", strSql.c_str());
	}

	sqlite3_finalize(stmt);
	return brow;
}

int CSqliteHandler::insertUserData(std::string strMAC, std::string strAccount, std::string strPassword,
		std::string strToken)
{
	if (getUserAuth(strMAC))
		return TRUE;

	int nRet = FALSE;
	string strSql = "INSERT INTO user(mac, account, password, token, created_date) values('" + strMAC + "','"
			+ strAccount + "','" + strPassword + "','" + strToken + "',datetime());";

	if ( SQLITE_OK == sqlExec(dbUser, strSql.c_str()))
	{
		nRet = TRUE;
	}
	return nRet;
}

int CSqliteHandler::updateUserAccount(std::string strMAC, std::string strAccount)
{
	if (!getUserAuth(strMAC))
		return FALSE;

	int nRet = FALSE;
	string strSql = "UPDATE user SET account = '" + strAccount + "' WHERE mac = '" + strMAC + "';";

	if ( SQLITE_OK == sqlExec(dbUser, strSql.c_str()))
	{
		nRet = TRUE;
	}
	return nRet;
}
