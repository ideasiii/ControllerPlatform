/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"
#include <string>

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
	void syncTrackerUser();
	void syncTrackerData();
	std::string getMysqlLastDate();

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
	void onTimer(int nId);

private:
	explicit CController();
	CMysqlHandler *mysql;
	CMongoDBHandler *mongo;
	volatile int mnBusy;

};
