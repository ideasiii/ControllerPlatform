/*
 * LogHandler.h
 *
 *  Created on: 2016年5月12日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <list>

class CThreadHandler;

class LogHandler
{
public:
	static LogHandler* getInstance();
	virtual ~LogHandler();
	void run();
	void setLogPath(std::string strPath);

private:
	LogHandler();
	CThreadHandler *tdExportLog;
};

extern void _log(const char* format, ...);
extern void _setLogPath(const char *ppath);
extern void _close();
extern void _error(const char* format, ...);
