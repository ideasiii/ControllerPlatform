/*
 * CManager.cpp
 *
 *  Created on: 2017年8月18日
 *      Author: jugo
 */

#include "CManager.h"
#include "CProcessManager.h"
#include "LogHandler.h"
#include "utility.h"
#include "CMysqlHandler.h"

#define TIMER_DB_CONNECT		5
#define TIMER_ID_DB_CONN		66678

using namespace std;

CManager::CManager(CObject *object) :
		processmanager(0), mysql(0)
{
	mpController = object;
	processmanager = new CProcessManager;
	mysql = new CMysqlHandler;
}

CManager::~CManager()
{
	killTimer(TIMER_ID_DB_CONN);
	delete processmanager;
	delete mysql;
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
			mapInfo["Name"] = *cit_set;
			for(cit_map = mapInfo.begin(); mapInfo.end() != cit_map; ++cit_map)
			{
				// Check Thread Count
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
				//_log("[CManager] checkProcess key: %s  value: %s", cit_map->first.c_str(), cit_map->second.c_str());
			}
			insertDB(mapInfo);
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

	clearDB();

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

void CManager::insertDB(map<string, string> &mapData)
{
	int Pid = 0, PPid = 0, Threads = 0, VmData = 0, VmExe = 0, VmLck = 0, VmLib = 0, VmPTE = 0, VmPeak = 0, VmRSS = 0,
			VmSize = 0, VmStk = 0, VmSwap = 0, nonvoluntary_ctxt_switches = 0, voluntary_ctxt_switches = 0;
	string strSQL;
	map<string, string>::const_iterator cit_map;

	if(!mysql->isValid())
	{
		if(!connectDB())
			return;
	}

	convertFromString(Pid, mapData["Pid"]);
	convertFromString(PPid, mapData["PPid"]);
	convertFromString(Threads, mapData["Threads"]);
	convertFromString(VmData, mapData["VmData"]);
	convertFromString(VmExe, mapData["VmExe"]);
	convertFromString(VmLck, mapData["VmLck"]);
	convertFromString(VmLib, mapData["VmLib"]);
	convertFromString(VmPTE, mapData["VmPTE"]);
	convertFromString(VmPeak, mapData["VmPeak"]);
	convertFromString(VmRSS, mapData["VmRSS"]);
	convertFromString(VmSize, mapData["VmSize"]);
	convertFromString(VmStk, mapData["VmStk"]);
	convertFromString(VmSwap, mapData["VmSwap"]);
	convertFromString(nonvoluntary_ctxt_switches, mapData["nonvoluntary_ctxt_switches"]);
	convertFromString(voluntary_ctxt_switches, mapData["voluntary_ctxt_switches"]);

	strSQL =
			format(
					"INSERT INTO proc_status (Name,State,Pid,PPid,Threads,VmData,VmExe,VmLck,VmLib,VmPTE,VmPeak,VmRSS,VmSize,VmStk,VmSwap, nonvoluntary_ctxt_switches,voluntary_ctxt_switches) VALUES('%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
					mapData["Name"].c_str(), mapData["State"].c_str(), Pid, PPid, Threads, VmData, VmExe, VmLck, VmLib,
					VmPTE, VmPeak, VmRSS, VmSize, VmStk, VmSwap, nonvoluntary_ctxt_switches, voluntary_ctxt_switches);
	mysql->sqlExec(strSQL);
}

void CManager::clearDB()
{
	if(!mysql->isValid())
	{
		if(!connectDB())
			return;
	}

	mysql->sqlExec("delete from proc_status where to_days(now()) - to_days(create_date) > 30");
}

void CManager::onTimer(int nId)
{
	switch(nId)
	{
	case TIMER_ID_DB_CONN:
		killTimer(TIMER_ID_DB_CONN);
		if(!connectDB())
			setTimer(TIMER_ID_DB_CONN, TIMER_DB_CONNECT, TIMER_DB_CONNECT);
		break;
	}
}

bool CManager::connectDB()
{
	if(!mysql->isValid())
	{
		if(mysql->connect("127.0.0.1", "monitor", "tracker", "ideas123!", "5"))
			return true;
		_log("[CManager] connectDB connect fail: %s", mysql->getLastError().c_str());
	}
	return false;
}
