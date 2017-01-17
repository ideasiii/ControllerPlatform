/*
 * CSqliteHandler.h
 *
 *  Created on: 2015年10月27日
 *      Author: Louis Ju
 */

#pragma once

#include <string>
#include <list>

class sqlite3;
class JSONArray;

class CSqliteHandler
{
public:
	explicit CSqliteHandler();
	virtual ~CSqliteHandler();

	int sqlExec(std::string strSQL);
	int connectDB(std::string strDBPath, std::list<std::string> listTable);
	int connectDB(std::string strDBPath);
	int query(std::string strSQL, JSONArray &jsonArray);
	void close();

private:
	sqlite3 *database;

};