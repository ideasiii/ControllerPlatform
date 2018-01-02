/*
 * CMysqlHandler.h
 *
 *  Created on: 2017年2月13日
 *      Author: jugo
 */

#pragma once

#include <mysql/mysql.h>
#include <string>
#include <list>
#include <map>
#include <set>

class CMysqlHandler
{
public:
	CMysqlHandler();
	virtual ~CMysqlHandler();
	int connect(std::string strHost, std::string strDB, std::string strUser, std::string strPassword,
			const char *szConnTimeout = "5");
	void close();
	int sqlExec(std::string strSQL);
	int query(std::string strSQL, std::list<std::map<std::string, std::string> > &listRest);
	std::string getLastError();
	int getLastErrorNo();
	int getFields(std::string strTableName, std::set<std::string> &sFields);
	bool isValid();

private:
	void setError(std::string strMsg);

private:
	std::string mstrLastError;
	int mnLastErrorNo;
	MYSQL *mpMySQL;
};
