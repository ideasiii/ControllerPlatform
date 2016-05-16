/*
 * LogHandler.h
 *
 *  Created on: 2016年5月12日
 *      Author: Jugo
 */

#pragma once

#include <string>

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
		std::string mstrLogPath;
};

extern void log(const char* format, ...);

