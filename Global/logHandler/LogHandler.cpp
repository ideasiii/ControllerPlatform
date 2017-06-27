/*
 * LogHandler.cpp
 *
 *  Created on: 2016年5月12日
 *      Author: root
 */

#include <syslog.h>
#include <fstream>
#include <stdio.h>
#include <cstdarg> // for Variable-length argument
#include "LogHandler.h"
#include "common.h"
#include "utility.h"

using namespace std;

static fstream fs;
static string mstrLogPath;
static string mstrLogDate = "2015-07-27";

extern char *__progname;

inline void writeLog(int nSize, const char *pLog)
{
	string strCurrentDate = currentDate();

	if(0 != mstrLogDate.compare(strCurrentDate) || !fs.is_open() || mstrLogPath.empty())
	{
		if(mstrLogPath.empty())
			mstrLogPath = format("/data/opt/tomcat/webapps/logs/%s.log", __progname);
		mstrLogDate = strCurrentDate;
		string strPath = format("%s.%s", mstrLogPath.c_str(), mstrLogDate.c_str());
		_close();
		fs.open(strPath.c_str(), fstream::in | fstream::out | fstream::app);
		fs.rdbuf()->pubsetbuf(0, 0);
		fs << currentDateTime() + " : [LogHandler] Open File: " + strPath << endl;
	}

	if(fs.is_open())
	{
		fs << pLog << endl;
	}
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

	writeLog(strLog.length(), strLog.c_str());

	//cout << strLog << endl;
	printf("%s\n", strLog.c_str());

}

void _setLogPath(const char *ppath)
{
	if(0 == ppath)
	{
		mstrLogPath = format("/data/opt/tomcat/webapps/logs/%s.log", __progname);
	}
	else
	{
		mstrLogPath = ppath;
		if(!mstrLogPath.empty())
			mkdirp(mstrLogPath);
	}
}

void _close()
{
	if(fs.is_open())
	{
		string strPath = format("%s.%s", mstrLogPath.c_str(), mstrLogDate.c_str());
		fs << currentDateTime() + " : [LogHandler] Close File: " + strPath << endl;
		fs.close();
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
