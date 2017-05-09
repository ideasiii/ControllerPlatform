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
#include "common.h"
#include "utility.h"

using namespace std;

static FILE *pstream = 0;
static string mstrLogPath = "./run.log";
static string mstrLogDate = "2015-07-27";

volatile bool bBusy = false;

inline void writeLog(int nSize, const char *pLog)
{
	char mbstr[16];
	std::time_t t;
	t = std::time( NULL);
	memset(mbstr, 0, 16);
	std::strftime(mbstr, 16, "%Y-%m-%d", std::localtime(&t));

	if(0 < strlen(mbstr) && 0 != mstrLogDate.compare(mbstr) && !bBusy)
	{
		bBusy = true;
		_close();
		mstrLogDate = mbstr;
		string strPath = format("%s.%s", mstrLogPath.c_str(), mbstr);
		pstream = fopen(strPath.c_str(), "a+");
		if(pstream)
		{
			if(0 != setvbuf(pstream, NULL, _IONBF, 0))
			{
				printf("[LogHandler] setvbuf fail \n");
			}
			printf("[LogHandler] Open Log File Success, Path: %s\n", strPath.c_str());
		}
		bBusy = false;
	}

	if(pstream && !bBusy)
		fwrite(pLog, 1, nSize, pstream);
	else
		_error("[LogHandler] _log Busy can't write log: %s", pLog);
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

	strLog = currentDateTime() + " : " + strLog + "\n";

	if(!mstrLogPath.empty() && pstream)
	{
		writeLog(strLog.length(), strLog.c_str());
	}

	printf("%s", strLog.c_str());

}

void _setLogPath(const char *ppath)
{
	if(0 == ppath)
	{
		mstrLogPath = "./run.log";
	}
	else
	{
		mstrLogPath = ppath;
		mkdirp(mstrLogPath);
	}
}

void _close()
{
	if(0 != pstream)
	{
		fclose(pstream);
		pstream = 0;
		printf("[LogHandler] Log File Closed\n");
	}
}

void _error(const char* format, ...)
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

	strLog = currentDateTime() + " : " + strLog + "\n";

	FILE *perr = fopen("error.log", "a+");
	if(perr)
	{
		fwrite(strLog.c_str(), 1, strLog.length(), perr);
		fclose(perr);
	}
}
