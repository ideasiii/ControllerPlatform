/*
 * LogHandler.cpp
 *
 *  Created on: 2016年5月12日
 *      Author: root
 */
#include <string>
#include <list>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <cstdarg> // for Variable-length argument
#include "LogHandler.h"
#include "CThreadHandler.h"
#include "common.h"
#include "utility.h"

using namespace std;

static LogHandler* mInstance = 0;

void *threadExportLog(void *argv)
{
	LogHandler* ss = reinterpret_cast<LogHandler*>(argv);
	ss->run();
	return NULL;
}

LogHandler::LogHandler() :
		tdExportLog(new CThreadHandler), mstrLogPath("run.log")
{
	tdExportLog->createThread(threadExportLog, this);
}

LogHandler::~LogHandler()
{
	tdExportLog->threadCancel(tdExportLog->getThreadID());
	tdExportLog->threadJoin(tdExportLog->getThreadID());
	delete tdExportLog;
}

LogHandler* LogHandler::getInstance()
{
	if (0 == mInstance)
	{
		mInstance = new LogHandler();
	}
	return mInstance;
}

void LogHandler::setLogPath(std::string strPath)
{
	if (!strPath.empty() && 0 < strPath.length())
	{
		mstrLogPath = strPath;
		mkdirp(mstrLogPath);
		_log("[Log Agent] Create Log Path:%s", mstrLogPath.c_str());
	}
}

void LogHandler::run()
{
	extern list<string> extListLog;
	string strLog;
	int nCount = 0;
	int i = 0;
	FILE *pstream;
	char szPath[255];
	std::time_t t;
	char mbstr[16];

	while (1)
	{
		tdExportLog->threadSleep(5);
		nCount = extListLog.size();

		if (0 >= nCount)
			continue;

		t = std::time( NULL);
		memset(mbstr, 0, 100);
		std::strftime(mbstr, 100, "%Y-%m-%d", std::localtime(&t));

		memset(szPath, 0, 255);
		sprintf(szPath, "%s.%s", mstrLogPath.c_str(), mbstr);
		pstream = fopen(szPath, "a");

		for (i = 0; i < nCount; ++i)
		{
			strLog = *(extListLog.begin());
			extListLog.pop_front();

			if ( NULL != pstream)
			{
				fprintf(pstream, "%s\n", strLog.c_str());
				fflush(pstream);
			}
			else
			{
				printf("[Error] Log file path open fail!!\n");
			}
		}
		fclose(pstream);
	}
}

void _log(const char* format, ...)
{
	char szLog[2048];
	char dest[2000];
	char mbstr[24];

	std::time_t t = std::time(0);
	std::strftime(mbstr, sizeof(mbstr), "%Y/%m/%d %T", std::localtime(&t));

	va_list argptr;
	va_start(argptr, format);
	vsprintf(dest, format, argptr);
	va_end(argptr);
	sprintf(szLog, "%s : %-20s", mbstr, dest);

	extern list<string> extListLog;
	extListLog.push_back(szLog);

	cout << szLog << endl;
}

