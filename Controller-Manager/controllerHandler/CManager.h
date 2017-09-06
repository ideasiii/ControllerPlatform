/*
 * CManager.h
 *
 *  Created on: 2017年8月18日
 *      Author: root
 */

#pragma once

#include <map>
#include <string>
#include <set>
#include "CCmpServer.h"

class CProcessManager;
class CMysqlHandler;

typedef struct _MONITOR
{
	int nThread_Max;
	_MONITOR()
	{
		nThread_Max = 30;
	}
} MONITOR;

enum
{
	PROC_NOT_RUN = 0, PROC_THREAD_OVER
};

class CManager: public CCmpServer
{
public:
	explicit CManager(CObject *object);
	virtual ~CManager();
	int checkProcess();
	void psInstanceDump(int pid);
	void setThreadMax(int nMax);
	void addProcess(const char *szProcessName);
	int getProcessCount();

protected:
	void onTimer(int nId);

private:
	void insertDB(std::map<std::string, std::string> &mapData);
	void clearDB();
	bool connectDB();

private:
	CObject *mpController;
	CProcessManager *processmanager;
	CMysqlHandler *mysql;

private:
	MONITOR monitor;
	std::set<std::string> setProcessName;
};
