/*
 * CManager.cpp
 *
 *  Created on: 2017年8月18日
 *      Author: jugo
 */

#include <map>
#include <string>
#include "CManager.h"
#include "CProcessManager.h"
#include "LogHandler.h"
#include "utility.h"

using namespace std;

CManager::CManager(CObject *object) :
		processmanager(0)
{
	mpController = object;
	processmanager = new CProcessManager;
}

CManager::~CManager()
{
	delete processmanager;
}

int CManager::checkProcess()
{
	map<string, string> mapInfo;
	map<string, string>::const_iterator cit_map;
	set<string>::const_iterator cit_set;
	Message message;
	int nValue;

	for(cit_set = setProcessName.begin(); setProcessName.end() != cit_set; ++cit_set)
	{
		if(processmanager->psStatus(cit_set->c_str(), mapInfo))
		{
			for(cit_map = mapInfo.begin(); mapInfo.end() != cit_map; ++cit_map)
			{
				if(!cit_map->first.compare("Threads"))
				{
					convertFromString(nValue, cit_map->second.c_str());
					if(monitor.nThread_Max < nValue)
					{
						message.what = PROC_THREAD_OVER;
						message.strData = *cit_set;
						mpController->sendMessage(message);
						_log("[CManager] checkProcess !! Process %s Thread too Many: %d !!", cit_set->c_str(), nValue);
						return FALSE;
					}
				}
				//	_log("[CManager] checkProcess key: %s  value: %s", cit_map->first.c_str(), cit_map->second.c_str());
			}
		}
		else
		{
			message.what = PROC_NOT_RUN;
			message.strData = cit_set->c_str();
			mpController->sendMessage(message);
			_log("[CManager] checkProcess !! Process %s not Run !!", cit_set->c_str());
			return FALSE;
		}
	}

	return TRUE;

// R (running)", "S (sleeping)", "D (disk sleep)", "T (stopped)", "T(tracing stop)", "Z (zombie)", or "X (dead)"
}

void CManager::setThreadMax(int nMax)
{
	monitor.nThread_Max = nMax;
	_log("[CManager] setThreadMax set Thread Max: %d", monitor.nThread_Max);
}

void CManager::addProcess(const char *szProcessName)
{
	if(szProcessName)
	{
		setProcessName.insert(szProcessName);
	}
}

int CManager::getProcessCount()
{
	return setProcessName.size();
}
