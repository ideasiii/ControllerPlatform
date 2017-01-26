/*
 * CTransferTracker.h
 *
 *  Created on: 2017年1月19日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <set>

class CMongoDBHandler;
class CSqliteHandler;
class CPsqlHandler;

class CTransferTracker
{
public:
	explicit CTransferTracker();
	virtual ~CTransferTracker();
	int start();

private:
	std::string getPSqlLastDate(std::string strTableName);
	int syncColume();
	int syncData();

private:
	CMongoDBHandler *mongo;
	CSqliteHandler *sqlite;
	CPsqlHandler *psql;
	std::set<std::string> sPoyaFieldIos;
	std::set<std::string> sPoyaFieldAndroid;
};
