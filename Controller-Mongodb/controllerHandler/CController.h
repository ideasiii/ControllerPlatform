/*
 * Controller.h
 *
 *  Created on: 2016年5月9日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CTrackerServer;
class CMongoDBHandler;

class CController: public CApplication
{
	struct MONGODB_CONN
	{
		std::string strIP;
		std::string strPort;
	};
public:
	explicit CController();
	virtual ~CController();

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);
	void onTimer(int nId);

private:
	int startTrackerServer(const int nPort, const int nMsqId);
	int startMongoClient(const char *szIP, const char *szPort);
	std::string insertLog(const int nType, std::string strData);

private:
	CTrackerServer *trackerServer;
	CMongoDBHandler *mongodb;
	int mnMsqKey;
	MONGODB_CONN mongo_conn;
};
