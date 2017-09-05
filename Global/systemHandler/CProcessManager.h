/*
 * CProcessManager.h
 *
 *  Created on: 2017年9月1日
 *      Author: jugo
 */

#pragma once

#include <string>
#include <map>

typedef struct processinfo_proto
{
	std::string name;
	std::string owner;
	std::string status;
	unsigned int uid, pid, load, threadnum;
	unsigned long rss;
	unsigned long delta_utime;
	unsigned long delta_stime;
	double delta_load;
} process_info;

class CProcessManager
{
public:
	CProcessManager();
	virtual ~CProcessManager();
	int getPid(const char * szProcessName);
	void psInstanceDump(int pid, process_info &psinfo);
	void psInstanceDump(const char * szProcessName, process_info &psinfo);
	int psStatus(const char * szPsName, std::map<std::string, std::string> &mapInfo);
	std::string psOwner(int pid);
};
