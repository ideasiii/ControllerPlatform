/*
 * CSqliteHandler.h
 *
 *  Created on: 2015年10月27日
 *      Author: Louis Ju
 */

#pragma once

#include <list>
#include <string>

#define COLUME_INDEX_CONTROLLER_SOCKETFD	1

class sqlite3;

enum
{
	DB_CONTROLLER = 0, DB_MDM_ANDROID
};

enum
{
	R_SQLITE_OK = 0, /*success*/
	R_SQLITE_ERROR = 1, /* SQL error or missing database */
	R_SQLITE_INTERNAL = 2, /* Internal logic error in SQLite */
	R_SQLITE_PERM = 3, /* Access permission denied */
	R_SQLITE_ABORT = 4, /* Callback routine requested an abort */
	R_SQLITE_BUSY = 5, /* The database file is locked */
	R_SQLITE_LOCKED = 6, /* A table in the database is locked */
	R_SQLITE_NOMEM = 7,/* A malloc() failed */
	R_SQLITE_READONLY = 8, /* Attempt to write a readonly database */
	R_SQLITE_INTERRUPT = 9, /* Operation terminated by sqlite3_interrupt()*/
	R_SQLITE_IOERR = 10, /* Some kind of disk I/O error occurred */
	R_SQLITE_CORRUPT = 11, /* The database disk image is malformed */
	R_SQLITE_NOTFOUND = 12, /* Unknown opcode in sqlite3_file_control() */
	R_SQLITE_FULL = 13, /* Insertion failed because database is full */
	R_SQLITE_CANTOPEN = 14, /* Unable to open the database file */
	R_SQLITE_PROTOCOL = 15, /* Database lock protocol error */
	R_SQLITE_EMPTY = 16, /* Database is empty */
	R_SQLITE_SCHEMA = 17, /* The database schema changed */
	R_SQLITE_TOOBIG = 18, /* String or BLOB exceeds size limit */
	R_SQLITE_CONSTRAINT = 19, /* Abort due to constraint violation */
	R_SQLITE_MISMATCH = 20, /* Data type mismatch */
	R_SQLITE_MISUSE = 21, /* Library used incorrectly */
	R_SQLITE_NOLFS = 22, /* Uses OS features not supported on host */
	R_SQLITE_AUTH = 23, /* Authorization denied */
	R_SQLITE_FORMAT = 24, /* Auxiliary database format error */
	R_SQLITE_RANGE = 25, /* 2nd parameter to sqlite3_bind out of range */
	R_SQLITE_NOTADB = 26, /* File opened that is not a database file */
	R_SQLITE_NOTICE = 27, /* Notifications from sqlite3_log() */
	R_SQLITE_WARNING = 28, /* Warnings from sqlite3_log() */
	R_SQLITE_ROW = 100, /* sqlite3_step() has another row ready */
	R_SQLITE_DONE = 101 /* sqlite3_step() has finished executing */
};

class CSqliteHandler
{
public:
	virtual ~CSqliteHandler();
	static CSqliteHandler* getInstance();
	int openControllerDB(const char *dbPath);
	int openMdmAndroidDB(const char *dbPath);
	int controllerSqlExec(const char *szSql);
	int mdmAndroidSqlExec(const char *szSql);
	int mdmAndroidSqlExec(const char *szSql, std::list<std::string> &listValue, int nColumeIndex);
	int sqlExec(sqlite3 *db, const char *szSql);
	int sqlExec(sqlite3 *db, const char *szSql, std::list<std::string> &listValue, int nColumeIndex);
	int getControllerColumeValue(const char *szTable, const char *szColume, std::list<std::string> &listValue);
	int getControllerColumeValue(const char *szTable, const char *szColume, std::list<std::string> &listValue,
			const char *szValue);
	int getControllerColumeValueInt(const char *szSql, std::list<int> &listValue, int nColumeIndex);
	void close();

private:
	explicit CSqliteHandler();
	static CSqliteHandler* m_instance;
};
