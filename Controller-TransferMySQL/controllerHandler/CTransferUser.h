/*
 * CTransferUser.h
 *
 *  Created on: 2017年1月16日
 *      Author: Jugo
 */

#pragma once

#include <string>

class CMysqlHandler;
class CPsqlHandler;

class CTransferUser
{
public:
	explicit CTransferUser();
	virtual ~CTransferUser();
	int start();

private:
	std::string getMysqlLastDate();
	CMysqlHandler *pmysql;
	CPsqlHandler *ppsql;
};
