/*
 * CController.cpp
 *
 *  Created on: 2019年01月07日
 *      Author: Louis Ju
 */

#include <string>
#include <map>
#include "CController.h"
#include "common.h"
#include "event.h"
#include "packet.h"
#include "utility.h"
#include "CConfig.h"
#include <dirent.h>
#include <stdio.h>

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

void CController::onTimer(int nId)
{
	switch (nId)
	{
	case 666:
		killTimer(666);
		foldScan();
		break;
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

void CController::foldScan()
{
	DIR *dir;
	struct dirent *stdire;
	dir = opendir("/data/arx");
	if (dir)
	{
		while (0 != (stdire = readdir(dir)))
		{
			if ((strcmp(stdire->d_name, ".") == 0) || (strcmp(stdire->d_name, "..") == 0))
				continue;

			printf("%s\n", stdire->d_name);
		}
		closedir(dir);
	}
}
