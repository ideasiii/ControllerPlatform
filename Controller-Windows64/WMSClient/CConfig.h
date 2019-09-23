#pragma once

#include <string>
#include <map>
#include <list>

class CConfig
{
	typedef std::map<std::string, std::string> MAP_CONF_VALUE;
	typedef std::list<MAP_CONF_VALUE> LIST_CONF_MAP;
	typedef std::map<std::string, LIST_CONF_MAP> MAP_CONF;		// <section,list<name,value>>

public:
	explicit CConfig();
	virtual ~CConfig();
	int loadConfig(std::string strConf);
	void setConfig(std::string strSection, std::string strName, std::string strValue);
	std::string getValue(std::string strSection, std::string strName);
	bool fileExists(const std::string& filename);

private:
	int readConfig(std::string strConf);

private:
	MAP_CONF mapConf;
};

