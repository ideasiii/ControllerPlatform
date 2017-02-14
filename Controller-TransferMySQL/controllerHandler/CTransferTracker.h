/*
 * CTransferTracker.h
 *
 *  Created on: 2017年1月19日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <set>

class CTransferTracker
{
public:
	explicit CTransferTracker();
	virtual ~CTransferTracker();
	int start();

private:
	std::string getPSqlLastDate(std::string strTableName);
	int syncColume(std::string strTable, std::string strAppId);
	int syncData(std::string strTable, std::string strAppId);

};
