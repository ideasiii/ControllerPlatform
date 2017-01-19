/*
 * CSqliteHandler.h
 *
 *  Created on: 2015年10月27日
 *      Author: Louis Ju
 */

#pragma once

#include <string>
#include <list>
#include <set>

using namespace std;

class sqlite3;
class JSONArray;

class CSqliteHandler
{
public:
	explicit CSqliteHandler();
	virtual ~CSqliteHandler();

	int sqlExec(string strSQL);
	int connectDB(string strDBPath, list<std::string> listTable);
	int connectDB(string strDBPath);
	int query(string strSQL, JSONArray &jsonArray);
	int getFields(string strTableName, set<string> &sFields);
	void close();

private:
	sqlite3 *database;

};
