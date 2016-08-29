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
static sqlite3 *dbMdmAndroid = 0;				// For RDM

CSqliteHandler* CSqliteHandler::m_instance = 0;

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	int i;
	for (i = 0; i < argc; i++) {
		_DBG("[Sqlite] %s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

CSqliteHandler::CSqliteHandler() {
}

CSqliteHandler::~CSqliteHandler() {
	close();
}

int CSqliteHandler::openControllerDB(const char *dbPath) {
	int rc = sqlite3_open(dbPath, &dbController);
	int nRet = FALSE;

	if (rc) {
		_log("[Sqlite] Can't open controller database: %s",
				sqlite3_errmsg(dbController));
	} else {
		sqlExec(dbController, "DROP TABLE controller;");
		const char *sql =
				"CREATE TABLE IF NOT EXISTS controller(id CHAR(50) NOT NULL, socket_fd INT NOT NULL );";
		if ( SQLITE_OK == sqlExec(dbController, sql)) {
			nRet = TRUE;
		}
	}

	return nRet;
}

int CSqliteHandler::openMdmAndroidDB(const char *dbPath) {
	int rc = sqlite3_open(dbPath, &dbMdmAndroid);
	int nRet = FALSE;

	if (rc) {
		_log("[Sqlite] Can't open mdm_android database: %s",
				sqlite3_errmsg(dbMdmAndroid));
	} else
		nRet = TRUE;
	return nRet;
}

void CSqliteHandler::close() {
	sqlite3_close(dbController);
	sqlite3_close(dbMdmAndroid);
	dbController = 0;
	dbMdmAndroid = 0;
}

CSqliteHandler* CSqliteHandler::getInstance() {
	if (!m_instance) {
		m_instance = new CSqliteHandler();
	}

	return m_instance;
}

int CSqliteHandler::controllerSqlExec(const char *szSql) {
	return sqlExec(dbController, szSql);
}

int CSqliteHandler::mdmAndroidSqlExec(const char *szSql) {
	return sqlExec(dbMdmAndroid, szSql);
}

int CSqliteHandler::sqlExec(sqlite3 *db, const char *szSql) {
	char *zErrMsg = 0;
	_log("[Sqlite] SQL exec: %s", szSql);
	int rc = sqlite3_exec(db, szSql, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		_log("[Sqlite] SQL exec error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	return rc;
}

int CSqliteHandler::getControllerColumeValue(const char *szTable,
		const char *szColume, std::list<std::string> &listValue) {
	string strSql = "SELECT " + string(szColume) + " FROM " + string(szTable)
			+ ";";

	sqlite3_stmt * stmt;
	sqlite3_prepare(dbController, strSql.c_str(), strSql.size() + 1, &stmt,
			NULL); //preparing the statement

	int s;
	int row = 0;

	while (1) {
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW) {
			const unsigned char * text;
			text = sqlite3_column_text(stmt, 0);
			listValue.push_back(string((const char*) text));
			++row;
		} else if (s == SQLITE_DONE) {
			break;
		} else {
			_log("[Sqlite] SQL:%s exec fail", strSql.c_str());
			break;
		}
	}

	sqlite3_finalize(stmt);
	return row;
}

int CSqliteHandler::getControllerColumeValue(const char *szTable,
		const char *szColume, std::list<std::string> &listValue,
		const char *szValue) {
	string strSql = "SELECT " + string(szColume) + " FROM " + string(szTable)
			+ " WHERE " + string(szColume) + " = " + string(szValue) + ";";

	sqlite3_stmt * stmt;
	sqlite3_prepare(dbController, strSql.c_str(), strSql.size() + 1, &stmt,
			NULL); //preparing the statement

	int s;
	int row = 0;

	while (1) {
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW) {
			const unsigned char * text;
			text = sqlite3_column_text(stmt, 0);
			listValue.push_back(string((const char*) text));
			++row;
		} else if (s == SQLITE_DONE) {
			break;
		} else {
			_log("[Sqlite] SQL:%s exec fail", strSql.c_str());
			break;
		}
	}

	sqlite3_finalize(stmt);
	return row;
}

int CSqliteHandler::getControllerColumeValueInt(const char *szSql,
		std::list<int> &listValue, int nColumeIndex) {
	sqlite3_stmt * stmt;
	int row = 0;
	int s = -1;
	int nValue = -1;

	sqlite3_prepare(dbController, szSql, strlen(szSql) + 1, &stmt, NULL);

	while (1) {
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW) {
			nValue = sqlite3_column_int(stmt, nColumeIndex);
			listValue.push_back(nValue);
			++row;
		} else {
			if (s != SQLITE_DONE) {
				_log("[Sqlite] SQL:%s exec fail", szSql);
			}
			break;
		}
	}

	sqlite3_finalize(stmt);

	return row;
}

int CSqliteHandler::mdmAndroidSqlExec(const char *szSql,
		std::list<std::string> &listValue, int nColumeIndex) {
	return sqlExec(dbMdmAndroid, szSql, listValue, nColumeIndex);
}

int CSqliteHandler::sqlExec(sqlite3 *db, const char *szSql,
		std::list<std::string> &listValue, int nColumeIndex) {
	sqlite3_stmt * stmt;
	int row = 0;
	int s = -1;
	int nValue = -1;
	const unsigned char * text;

	sqlite3_prepare(db, szSql, strlen(szSql) + 1, &stmt, NULL);

	while (1) {
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW) {
			text = sqlite3_column_text(stmt, nColumeIndex);
			listValue.push_back(string((const char*) text));
			++row;
		} else {
			if (s != SQLITE_DONE) {
				_log("[Sqlite] SQL:%s exec fail", szSql);
			}
			break;
		}
	}

	sqlite3_finalize(stmt);

	return row;
}
