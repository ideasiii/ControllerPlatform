#pragma once

#include "stdafx.h"
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <sstream>      // std::stringstream, std::stringbuf
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <cstdarg>
#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <string>
#include <set>

using namespace std;

#define MAX (a,b) (((a) > (b)) ? (a) : (b))
#define MIN (a,b) (((a) < (b)) ? (a) : (b))

extern char* __progname;

template<class T>
string ConvertToString(T value)
{
	stringstream ss;
	ss << value;
	return ss.str();
}

template<class T>
void convertFromString(T& value, const std::string& s)
{
	std::stringstream ss(s);
	ss >> value;
}

static int spliteData(char* pData, const char* delim, vector<string>& vData)
{
	char* pch;

	pch = strtok(pData, delim);
	while (pch != NULL)
	{
		vData.push_back(string(pch));
		pch = strtok(NULL, delim);
	}

	return vData.size();
}

static int spliteData(char* pData, const char* delim, set<string>& setData)
{
	char* pch;

	pch = strtok(pData, delim);
	while (pch != NULL)
	{
		setData.insert(string(pch));
		pch = strtok(NULL, delim);
	}

	return setData.size();
}



static const string currentDateTime()
{
	time_t now = time(0);
	struct tm tstruct;
	char buf[24];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
	return buf;
}

static const void currentDateTimeNum(int& nYear, int& nMonth, int& nDay, int& nHour,
	int& nMinute, int& nSecond)
{
	time_t now = time(0);
	struct tm tstruct;

	tstruct = *localtime(&now);
	nYear = tstruct.tm_year + 1900;
	nMonth = tstruct.tm_mon + 1;
	nDay = tstruct.tm_mday;
	nHour = tstruct.tm_hour;
	nMinute = tstruct.tm_min;
	nSecond = tstruct.tm_sec;
	printf("%d %d %d %d %d %d\n", nYear, nMonth, nDay, nHour, nMinute, nSecond);
}

static const string currentDate()
{
	time_t now = time(0);
	struct tm tstruct;
	char buf[24];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
	return buf;
}

static const int currentDateNum()
{
	int nDate;
	time_t now = time(0);
	struct tm tstruct;
	char buf[24];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%04Y%02m%02d", &tstruct);
	convertFromString(nDate, buf);
	return nDate;
}

static string trim(string strSource)
{
	string strTmp;
	strTmp = strSource;
	strTmp.erase(0, strTmp.find_first_not_of(" \n\r\t"));
	strTmp.erase(strTmp.find_last_not_of(" \n\r\t") + 1);
	return strTmp;
}

static bool isValidStr(const char* szStr, int nMaxSize)
{
	if ((0 != szStr) && 0 < ((int)strlen(szStr)) && nMaxSize > ((int)strlen(szStr)))
		return true;
	else
		return false;
}

static long int nowSecond()
{
	time_t tnow;
	time(&tnow);
	return tnow;
}

// 將 i 的數值轉換為 16 進位表示法的字串
template<typename T>
static std::string numberToHex(T i)
{
	std::stringstream stream;
	stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << i;
	return stream.str();
}

static int getRand(int nMin, int nMax)
{
	srand(time(NULL));
	int x = rand() % (nMax - nMin + 1) + nMin;
	return x;
}

static string ReplaceAll(string str, const string& from, const string& to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}

static string getConfigFile()
{
	string strProcessName = __progname;
	size_t found = strProcessName.find_last_of("/\\");
	return strProcessName.substr(++found) + ".conf";
}
