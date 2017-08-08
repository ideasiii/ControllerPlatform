/*
 * CConfig.cpp
 *
 *  Created on: 2015年10月20日
 *      Author: Louis Ju
 */

#include "common.h"
#include "CConfig.h"
#include "utility.h"
#include "CConfigHandler.h"

using namespace std;

CConfig::CConfig()
{

}

CConfig::~CConfig()
{
	mapConf.clear();
}

int CConfig::loadConfig(std::string strConf)
{
	int nRet = FALSE;
	if(isValidStr(strConf.c_str(), 255))
	{
		if(0 < readConfig(strConf))
			nRet = TRUE;
	}
	return nRet;
}

int funcConfig(void *object, const char *section, const char *name, const char *value)
{
	int nRet = -1;
	CConfig *conf = reinterpret_cast<CConfig *>(object);
	conf->setConfig(section, name, value);
	return nRet;
}

int CConfig::readConfig(std::string strConf)
{
	int nRet = -1;

	CConfigHandler *configHandler = new CConfigHandler;
	nRet = configHandler->parse(strConf.c_str(), funcConfig, this);
	delete configHandler;
	return nRet;
}

void CConfig::setConfig(string strSection, string strName, string strValue)
{
	MAP_CONF_VALUE mapData;
	mapData.insert(std::make_pair(strName, strValue));

	if(mapConf.find(strSection) != mapConf.end())
	{
		mapConf[strSection].push_back(mapData);
	}
	else
	{
		LIST_CONF_MAP listConf;
		listConf.push_back(mapData);
		mapConf.insert(std::make_pair(strSection, listConf));
	}
}

string CConfig::getValue(string strSection, string strName)
{
	string strValue;

	if(mapConf.find(strSection) != mapConf.end())
	{
		for(LIST_CONF_MAP::iterator i = mapConf[strSection].begin(); i != mapConf[strSection].end(); ++i)
		{
			//auto find = (*i).find(trim(strName));
			if((*i).find(trim(strName)) != (*i).end())
			{
				strValue = (*i)[trim(strName)];
				break;
				//return trim(find->second);
			}
		}
	}
	return strValue;
}

