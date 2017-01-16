/*
 * CTransferUser.h
 *
 *  Created on: 2017年1月16日
 *      Author: Jugo
 */

#pragma once

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
	CSqliteHandler *sqlite;
	CPsqlHandler *psql;
};
