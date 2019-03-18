/*
 * CFileHandler.cpp
 *
 *  Created on: 2017年5月26日
 *      Author: Jugo
 */

#include <string.h> /* for strerror */
#include <fstream>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "CFileHandler.h"
#include "LogHandler.h"
#include "utility.h"

using namespace std;

CFileHandler::CFileHandler() :
		mpController(0)
{

}

CFileHandler::CFileHandler(CObject *object)
{
	mpController = object;
}

CFileHandler::~CFileHandler()
{

}

unsigned int CFileHandler::readAllLine(const char *szFile, vector<string> &setData)
{
	if (szFile)
	{
		ifstream file(szFile);
		string str;
		if (file.is_open())
		{
			while (getline(file, str))
				setData.push_back(str);
			file.close();
		}
	}
	return setData.size();
}

unsigned int CFileHandler::readContent(const char *szFile, string &strContent, bool bTrim)
{
	strContent.clear();

	if (szFile)
	{
		ifstream file(szFile);
		string str;
		if (file.is_open())
		{
			while (getline(file, str))
			{
				if (bTrim)
				{
					strContent.append(trim(str));
				}
				else
				{
					strContent.append(str);
				}
			}
			file.close();
		}
	}
	return strContent.length();
}

unsigned int CFileHandler::readPath(const char *szPath, vector<string> &setData)
{
	DIR *dp;
	struct dirent *dirp;

	if (szPath)
	{
		if (!(dp = opendir(szPath)))
		{
			_log("[CFileHandler] readPath Error: %s", strerror(errno));
		}
		else
		{
			while ((dirp = readdir(dp)))
			{
				if (dirp->d_type != DT_DIR)
					setData.push_back(string(dirp->d_name));
			}
			closedir(dp);
		}
	}
	return setData.size();
}

unsigned int CFileHandler::readAllLine(const char *szFile, std::set<std::string> &setData)
{
	if (szFile)
	{
		ifstream file(szFile);
		string str;
		if (file.is_open())
		{
			while (getline(file, str))
				setData.insert(str);
			file.close();
		}
	}
	return setData.size();
}

unsigned int CFileHandler::readPath(const char *szPath, std::set<std::string> &setData)
{
	DIR *dp;
	struct dirent *dirp;

	if (szPath)
	{
		if (!(dp = opendir(szPath)))
		{
			_log("[CFileHandler] readPath Error: %s", strerror(errno));
		}
		else
		{
			while ((dirp = readdir(dp)))
			{
				if (dirp->d_type != DT_DIR)
					setData.insert(string(dirp->d_name));
			}
			closedir(dp);
		}
	}
	return setData.size();
}

int CFileHandler::copyFile(const char* src, const char* des)
{
	int nRet = 0;
	FILE* pSrc = NULL, *pDes = NULL;
	pSrc = fopen(src, "r");
	pDes = fopen(des, "w+");

	if (pSrc && pDes)
	{
		int nLen = 0;
		char szBuf[1024] = { 0 };
		while ((nLen = fread(szBuf, 1, sizeof szBuf, pSrc)) > 0)
		{
			fwrite(szBuf, 1, nLen, pDes);
		}
	}
	else
		nRet = -1;

	if (pSrc)
		fclose(pSrc), pSrc = NULL;

	if (pDes)
		fclose(pDes), pDes = NULL;

	return nRet;
}

