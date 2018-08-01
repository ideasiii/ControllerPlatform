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

unsigned int CFileHandler::readAllLine(const char *szFile, set<string> &setData)
{
	if(szFile)
	{
		ifstream file(szFile);
		string str;
		if(file.is_open())
		{
			while(getline(file, str))
				setData.insert(str);
			file.close();
		}
	}
	return setData.size();
}

unsigned int CFileHandler::readContent(const char *szFile, string &strContent, bool bTrim)
{
	strContent.clear();

	if(szFile)
	{
		ifstream file(szFile);
		string str;
		if(file.is_open())
		{
			while(getline(file, str))
			{
				if(bTrim)
				{
					strContent.append(trim(str));
					//_log("[CFileHandler] readContent line: %s", trim(str).c_str());
				}
				else
				{
					strContent.append(str);
					//_log("[CFileHandler] readContent line: %s", str.c_str());
				}
			}
			//_log("[CFileHandler] readContent Content: %s", strContent.c_str());
			file.close();
		}
	}
	return strContent.length();
}

unsigned int CFileHandler::readPath(const char *szPath, set<string> &setData)
{
	DIR *dp;
	struct dirent *dirp;

	if(szPath)
	{
		if(!(dp = opendir(szPath)))
		{
			_log("[CFileHandler] readPath Error: %s", strerror(errno));
		}
		else
		{
			while((dirp = readdir(dp)))
			{
				if(dirp->d_type != DT_DIR)
					setData.insert(string(dirp->d_name));
			}
			closedir(dp);
		}
	}
	return setData.size();
}

