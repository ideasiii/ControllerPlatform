/*
 * CMysqlHandler.h
 *
 *  Created on: 2017年2月13日
 *      Author: jugo
 */

#pragma once

#include<mysql/mysql.h>
#include <string>
#include <list>
#include <map>

class CMysqlHandler
{
public:
	CMysqlHandler();
	virtual ~CMysqlHandler();
	int connect(std::string strHost, std::string strDB, std::string strUser, std::string strPassword);
	void close();
	int sqlExec(std::string strSQL);
	int query(std::string strSQL, std::list<std::map<std::string, std::string> > &listRest);
	std::string getLastError();

private:
	void setError(std::string strMsg);

private:
	MYSQL *mpMySQL;
	std::string mstrLastError;
};
