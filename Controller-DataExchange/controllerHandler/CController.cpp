/*
 * CController.cpp
 *
 *  Created on: 2019年01月07日
 *      Author: Louis Ju
 */

#include "CController.h"
#include "common.h"
#include "event.h"
#include "packet.h"
#include "utility.h"
#include "CConfig.h"
#include "CFileHandler.h"
#include "CString.h"
#include <dirent.h>
#include <stdio.h>
#include <set>
#include <thread>
#include <map>
#include <unistd.h>
#include <signal.h>
#include <cstdint>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <iostream>     // std::streambuf, std::cout
#include <fstream>      // std::ofstream
#include <algorithm>    // std::remove

#define PATH_FOLDER		"/data/arx"
#define PATH_FINISH		"/data/arx/finished"

using namespace std;

CController::CController() :
		mnMsqKey(-1)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{

	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_DATAEXT;
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nRet;
	int nPort;
	CConfig *config;
	string strConfPath;
	string strValue;

	nRet = FALSE;
	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if (strConfPath.empty())
		return FALSE;

	setTimer(666, 3, 3);

	return nRet;
}

void CController::accessFile()
{
	CString strPath;
	CFileHandler fileHandler;
	vector<string> vFileList;
	vector<string> vCsv;

	foldScan(PATH_FOLDER, vFileList);

	for (vector<string>::iterator vecit = vFileList.begin(); vFileList.end() != vecit; ++vecit)
	{
		strPath.format("%s/%s", PATH_FOLDER, vecit->c_str());
		_log("[CController] accessFile File name: %s", strPath.getBuffer());
		vCsv.clear();
		fileHandler.readAllLine(strPath.getBuffer(), vCsv);
		if (1 < vCsv.size())
		{
			if (0 == importDB(strPath))
			{
				moveFile(vecit->c_str());
			}
		}
	}
}

int CController::importDB(const char * szPath)
{
	pid_t pid;
	int status = -1;

	if (szPath)
	{
		char *arg_list[] = { const_cast<char*>("mongoimport"), const_cast<char*>("--db"), const_cast<char*>("findata"),
				const_cast<char*>("--collection"), const_cast<char*>("csv"), const_cast<char*>("--type"),
				const_cast<char*>("csv"), const_cast<char*>("--headerline"), const_cast<char*>("--ignoreBlanks"),
				const_cast<char*>("--file"), const_cast<char*>(szPath), NULL };

		status = posix_spawn(&pid, "/usr/bin/mongoimport", NULL, NULL, arg_list, environ);
		if (status == 0)
		{
			_log("[CController] importDB posix_spawn Child pid: %i", pid);
			if (waitpid(pid, &status, 0) != -1)
			{
				_log("[CController] importDB Child exited with status %i", status);
			}
			else
			{
				_log("[CController] importDB waitpid Error");
			}
		}
		else
		{
			_log("[CController] importDB Error posix_spawn: %s", strerror(status));
		}
	}
	return status;
}

void CController::moveFile(const char * szPath)
{
	CString strOld;
	CString strNew;

	time_t rawtime;
	time(&rawtime);
	strOld.format("%s/%s", PATH_FOLDER, szPath);
	strNew.format("%s/%ld_%s", PATH_FINISH, rawtime, szPath);

	ifstream ifs(strOld.getBuffer(), ios::in | ios::binary);
	ofstream ofs(strNew.getBuffer(), ios::out | ios::binary);
	ofs << ifs.rdbuf();
	std::remove(strOld.getBuffer());
}

void CController::onTimer(int nId)
{
	if (666 == nId)
	{
		killTimer(nId);
		accessFile();
		setTimer(666, 1, 3);
		return;
	}
}

int CController::onFinish(void* nMsqKey)
{
	killTimer(666);
	return TRUE;
}

void CController::onHandleMessage(Message &message)
{

}

void CController::foldScan(const char * szFolderPath, vector<string> & vFileList)
{
	CFileHandler fileHandler;
	vector<string> vFiles;
	int nIndex;

	if (fileHandler.readPath(szFolderPath, vFiles))
	{
		for (vector<string>::iterator it = vFiles.begin(); vFiles.end() != it; ++it)
		{
			nIndex = it->rfind(".");
			if ((int) string::npos != nIndex)
			{
				++nIndex;
				if (!it->substr(nIndex).compare("csv") || !it->substr(nIndex).compare("CSV"))
				{
					vFileList.push_back(*it);
				}
			}
		}
	}
	vFiles.clear();
}
