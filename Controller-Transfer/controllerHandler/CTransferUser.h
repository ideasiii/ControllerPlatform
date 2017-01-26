/*
 * CTransferUser.h
 *
 *  Created on: 2017年1月16日
 *      Author: Jugo
 */

#pragma once

#include <string>

class CSqliteHandler;
class CPsqlHandler;

class CTransferUser
{
public:
	explicit CTransferUser();
	virtual ~CTransferUser();
	int start();
	void stop();

private:
	std::string getPSqlLastDate();

private:
	CSqliteHandler *sqlite;
	CPsqlHandler *psql;
};
