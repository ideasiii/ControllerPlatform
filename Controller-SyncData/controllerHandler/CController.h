/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"
#include <string>
#include <set>
class CMysqlHandler;
class CMongoDBHandler;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int start();
	void stop();
	void setMysqlSource(const char *szHost, const char *szPort, const char *szDB, const char *szUser,
			const char *szPassword);
	void setMysqlDestination(const char *szHost, const char *szPort, const char *szDB, const char *szUser,
			const char *szPassword);
	void setMongoDB(const char *szHost, const char *szPort);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
	void onTimer(int nId);

private:
	void syncTrackerUser();
	void syncTrackerData();
	std::string getMysqlLastDate(const char *szTable);
	int syncColume(std::string strTable, std::string strAppId);
	int getDestFields(std::string strTableName, std::set<std::string> &sFields);
	int getFields(std::string strAppId, std::set<std::string> &sFields);
	int syncData(std::string strTable, std::string strAppId);

private:
	explicit CController();
	CMysqlHandler *mysql;
	CMongoDBHandler *mongo;
	volatile int mnBusy;

};
