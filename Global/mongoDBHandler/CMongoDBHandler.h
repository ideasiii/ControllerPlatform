/*
 * CMongoDBHandler.h
 *
 *  Created on: 2015年12月01日
 *      Author: Louis Ju
 */

#pragma once
#include <string>
#include <map>
#include <list>

using namespace std;

class CMongoDBHandler
{
public:
	static CMongoDBHandler* getInstance();
	virtual ~CMongoDBHandler();

	int connectDB();
	int connectDB(string strIP, string strPort);
	int connectDB(string strIP, string strPort, string strDBName, string strUser, string strPasswd);
	void close();
	void insert(string strDB, string strCollection, map<string, string> &mapData);
	void insert(string strDB, string strCollection, string strColumn, string strValue);
	std::string insert(string strDB, string strCollection, string strJSON);
	int query(string strDB, string strCollection, string strField, string strCondition, list<string> &listJSON);
	bool isValid();
	int query(string strDB, string strCollection, string strField, string strFilter, string strCondition,
			list<string> &listJSON);

private:
	explicit CMongoDBHandler();
	static CMongoDBHandler* mInstance;
};
