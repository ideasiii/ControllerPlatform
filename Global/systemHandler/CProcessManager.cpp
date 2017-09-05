/*
 * CProcessManager.cpp
 *
 *  Created on: 2017年9月1日
 *      Author: jugo
 */

#include <set>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/klog.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>
#include <vector>
#include "CProcessManager.h"
#include "common.h"
#include "utility.h"
#include "LogHandler.h"
#include "CFileHandler.h"

using namespace std;

CProcessManager::CProcessManager()
{

}

CProcessManager::~CProcessManager()
{

}

int CProcessManager::getPid(const char * szProcessName)
{
	char pidline[1024];
	char *pid;
	int nPid;
	int pidno[64];
	string strCommand;

	if(!szProcessName)
		return 0;

	nPid = -1;
	strCommand = format("pidof -s %s", szProcessName);
	FILE *fp = popen(strCommand.c_str(), "r");
	fgets(pidline, 1024, fp);
	pclose(fp);

	convertFromString(nPid, pidline);

	_log("[CProcessManager] getPid get %s PID: %d", szProcessName, nPid);
	return nPid;
}

void CProcessManager::psInstanceDump(int pid, process_info &psinfo)
{
	CFileHandler fh;
	struct stat stats;
	string strPsPath;
	string strContent;
	struct passwd *pw;
	vector<string> vData;
	size_t pos;

	memset(&psinfo, 0, sizeof(process_info));
	psinfo.pid = pid;

	strPsPath = format("/proc/%d", pid);
	stat(strPsPath.c_str(), &stats);

	psinfo.uid = (int) stats.st_uid;

	pw = getpwuid(stats.st_uid);
	if(pw == 0)
	{
		psinfo.owner = format("%d", (int) stats.st_uid);
	}
	else
	{
		psinfo.owner = pw->pw_name;
	}

	strPsPath = format("/proc/%d/stat", pid);
	fh.readContent(strPsPath.c_str(), strContent);
	_log("[CProcessManager] psInstanceDump proc stat: %s", strContent.c_str());

	spliteData(const_cast<char*>(strContent.c_str()), " ", vData);

//	convertFromString(psinfo.delta_stime, vData.at(stime));
//	convertFromString(psinfo.delta_utime, vData.at(utime));
//	convertFromString(psinfo.rss, vData.at(utime));
	psinfo.delta_load = (psinfo.delta_stime + psinfo.delta_utime) * 100;

	//======= Get Process Name ========//
	strPsPath = format("/proc/%d/cmdline", pid);
	fh.readContent(strPsPath.c_str(), strContent);
	pos = strContent.find_last_of("/");
	if(string::npos != pos)
	{
		strContent = strContent.substr(++pos);
	}
	psinfo.name = strContent;

}

void CProcessManager::psInstanceDump(const char * szProcessName, process_info &psinfo)
{
	if(!szProcessName)
		return;

	psInstanceDump(getPid(szProcessName), psinfo);
}

int CProcessManager::psStatus(const char * szPsName, std::map<std::string, std::string> &mapInfo)
{
	int nPid;
	CFileHandler fh;
	string strPsPath;
	string strContent;
	vector<string> vData;
	set<string> setData;
	set<string>::const_iterator cit_set;
	if(!szPsName)
		return FALSE;

	nPid = getPid(szPsName);
	if(0 >= nPid)
		return FALSE;

	strPsPath = format("/proc/%d/status", nPid);

	fh.readAllLine(strPsPath.c_str(), setData);

	for(cit_set = setData.begin(); setData.end() != cit_set; ++cit_set)
	{
		vData.clear();
		spliteData(const_cast<char*>(cit_set->c_str()), ":", vData);
		mapInfo[trim(vData.at(0))] = trim(vData.at(1));
	}

	return TRUE;
}

std::string CProcessManager::psOwner(int pid)
{
	string strOwner;
	CFileHandler fh;
	struct stat stats;
	string strPsPath;
	struct passwd *pw;

	strPsPath = format("/proc/%d", pid);
	stat(strPsPath.c_str(), &stats);

	pw = getpwuid(stats.st_uid);
	if(pw == 0)
	{
		strOwner = format("%d", (int) stats.st_uid);
	}
	else
	{
		strOwner = pw->pw_name;
	}
	return strOwner;
}
