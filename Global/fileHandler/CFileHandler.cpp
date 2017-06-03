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

unsigned int CFileHandler::readAllLine(const char *szFile, std::set<std::string> &setData)
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

unsigned int CFileHandler::readPath(const char *szPath, std::set<std::string> &setData)
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

