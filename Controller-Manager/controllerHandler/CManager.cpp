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

void CManager::checkProcess()
{
	map<string, string> mapInfo;
	map<string, string>::const_iterator cit_map;

	processmanager->psStatus("controller-semantic", mapInfo);
	for(cit_map = mapInfo.begin(); mapInfo.end() != cit_map; ++cit_map)
	{
		_log("[CManager] checkProcess key: %s  value: %s", cit_map->first.c_str(), cit_map->second.c_str());
	}

	// R (running)", "S (sleeping)", "D (disk sleep)", "T (stopped)", "T(tracing stop)", "Z (zombie)", or "X (dead)"
}

void CManager::setThreadMax(int nMax)
{
	monitor.nThread_Max = nMax;
	_log("[CManager] setThreadMax set Thread Max: %d", monitor.nThread_Max);
}
