/*
 * LogHandler.cpp
 *
 *  Created on: 2016年5月12日
 *      Author: root
 */

#include <list>
#include <ctime>
#include <stdio.h>
#include <cstdarg> // for Variable-length argument
#include "LogHandler.h"
#include "CThreadHandler.h"
#include "common.h"
#include "utility.h"

using namespace std;

static LogHandler* mInstance = 0;
/* Log list*/
__attribute__ ((unused)) static list<string> extListLog;

static FILE *pstream = 0;
static string mstrLogPath;
static string mstrLogDate = "2015-07-27";
static char mbstr[16];


inline void writeLog(int nSize, const char *pLog)
{
	std::time_t t;
	t = std::time( NULL);
	memset(mbstr, 0, 16);
	std::strftime(mbstr, 16, "%Y-%m-%d", std::localtime(&t));

	if (0 != mstrLogDate.compare(mbstr))
	{
		if (0 != pstream)
			fclose(pstream);

		mstrLogDate = mbstr;
		string strPath = format("%s.%s", mstrLogPath.c_str(), mbstr);
		pstream = fopen(strPath.c_str(), "a");
		if (pstream)
			printf("[LogHandler] Open Log File Success, Path: %s\n", strPath.c_str());
	}

	if (0 == pstream)
	{
		printf("[LogHandler] Error, pstream invalid\n");
		return;
	}

	flockfile(pstream);
	fwrite(pLog, 1, nSize, pstream);
	fwrite("\n", 1, 1, pstream);
	//fprintf(pstream, "%s\n", pLog);
	fflush(pstream);
	funlockfile(pstream);
}

void _log(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	int size = vsnprintf(0, 0, format, vl) + sizeof('\0');
	va_end(vl);

	char buffer[size];

	va_start(vl, format);
	size = vsnprintf(buffer, size, format, vl);
	va_end(vl);

	string strLog = string(buffer, size);

	strLog = currentDateTime() + " : " + strLog;

	if (mstrLogPath.empty())
	{
		printf("[LogHandler] Log Path not Setting\n");
	}
	else
	{
		//writeLog(strLog.length(), strLog.c_str());
	}

	printf("%s\n", strLog.c_str());

}

void _setLogPath(const char *ppath)
{
	if (0 == ppath)
	{
		mstrLogPath = "./run.log";
		return;
	}

	mstrLogPath = ppath;
	mkdirp(mstrLogPath);
	_log("[Log Agent] Create Log Path:%s", mstrLogPath.c_str());
}

void _close()
{
	if (0 != pstream)
	{
		fclose(pstream);
		pstream = 0;
	}
}

//=============================Deprecated================================================//

void *threadExportLog(void *argv)
{
	LogHandler* ss = reinterpret_cast<LogHandler*>(argv);
	ss->run();
	return NULL;
}

LogHandler::LogHandler() :
		tdExportLog(new CThreadHandler)
{
	tdExportLog->createThread(threadExportLog, this);
}

LogHandler::~LogHandler()
{
	tdExportLog->threadCancel(tdExportLog->getThreadID());
	tdExportLog->threadJoin(tdExportLog->getThreadID());
	delete tdExportLog;
	fclose(pstream);
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
	string strLog;
	unsigned long nCount = 0;

	char szPath[255];
	std::time_t t;

	string strLogDate = "2015-07-27";

	while (1)
	{
		tdExportLog->threadSleep(1);
		nCount = extListLog.size();

		if (0 >= nCount)
			continue;

		t = std::time( NULL);
		memset(mbstr, 0, 16);
		std::strftime(mbstr, 16, "%Y-%m-%d", std::localtime(&t));

		if (0 != strLogDate.compare(mbstr))
		{
			if (0 != pstream)
				fclose(pstream);

			strLogDate = mbstr;
			memset(szPath, 0, 255);
			sprintf(szPath, "%s.%s", mstrLogPath.c_str(), mbstr);
			pstream = fopen(szPath, "a");
		}

		for (unsigned long i = 0; i < nCount; ++i)
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
	}
}

