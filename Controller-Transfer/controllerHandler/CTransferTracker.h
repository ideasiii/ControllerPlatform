/*
 * CTransferTracker.h
 *
 *  Created on: 2017年1月19日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <set>

using namespace std;

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
	string getPSqlLastDate();
	int syncColume();

private:
	CMongoDBHandler *mongo;
	CSqliteHandler *sqlite;
	CPsqlHandler *psql;
	set<string> sPoyaFieldIos;
	set<string> sPoyaFieldAndroid;

};
