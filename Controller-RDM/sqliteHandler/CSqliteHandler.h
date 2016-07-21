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
