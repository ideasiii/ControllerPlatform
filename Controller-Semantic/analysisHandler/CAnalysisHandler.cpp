/*
 * CAnalysisHandler.cpp
 *
 *  Created on: 2017年7月18日
 *      Author: Jugo
 */

#include <set>
#include <string>
#include "CAnalysisHandler.h"
#include "LogHandler.h"
#include "CConfig.h"
#include "CFileHandler.h"
#include "utility.h"

using namespace std;

CAnalysisHandler::CAnalysisHandler(const char *szConf)
{
	mbValid = loadConf(szConf);
	if(mbValid)
	{
		loadData();
	}
}

CAnalysisHandler::~CAnalysisHandler()
{

}

bool CAnalysisHandler::loadConf(const char *szConf)
{
	bool bResult;
	string strValue;
	CConfig *config;

	bResult = false;

	if(szConf)
	{
		_log("[CAnalysisHandler] CAnalysisHandler Load conf file: %s", szConf);

		config = new CConfig();
		if(config->loadConfig(szConf))
		{
			conf.strName = config->getValue("CONF", "name");
			strValue = config->getValue("CONF", "type");
			convertFromString(conf.nType, strValue);
			switch(conf.nType)
			{
			case CONF_TYPE_LOCAL_FILE:
				conf.uConf.conf_file.strPath = config->getValue("FILE", "path");
				conf.uConf.conf_file.strType = config->getValue("FILE", "type");
				bResult = true;
				break;
			case CONF_TYPE_DICTIONARY:
				break;
			default:
				bResult = false;
				_log("[CAnalysisHandler] CAnalysisHandler Unknow config type: %d", conf.nType);
				break;
			}
			_log("[CAnalysisHandler] CAnalysisHandler get conf name: %s type: %d", conf.strName.c_str(), conf.nType);
		}
		delete config;
	}

	return bResult;
}

void CAnalysisHandler::loadData()
{
	switch(conf.nType)
	{
	case CONF_TYPE_LOCAL_FILE:
		loadLocalFile();
		break;
	case CONF_TYPE_DICTIONARY:
		loadDictionary();
		break;
	}
}

void CAnalysisHandler::loadLocalFile()
{
	int nIndex;
	string strFileType;
	string strFileName;
	CFileHandler fh;
	set<string> setData;
	set<string>::const_iterator iter_set;
	LOCAL_DATA localData;

	fh.readPath(conf.uConf.conf_file.strPath.c_str(), setData);
	for(iter_set = setData.begin(); setData.end() != iter_set; ++iter_set)
	{
		nIndex = iter_set->rfind(".");
		if((int) string::npos != nIndex)
		{
			strFileType = iter_set->substr(nIndex + 1);
			if(!strFileType.compare(conf.uConf.conf_file.strType))
			{
				strFileName = iter_set->substr(0, nIndex);
				localData.clear();
				localData.strName = strFileName;
				localData.strPath = conf.uConf.conf_file.strPath + (*iter_set);
				localData.strType = conf.uConf.conf_file.strType;
				mapLocalData[strFileName] = localData;
			}
		}
	}

	for(map<string, LOCAL_DATA>::const_iterator it = mapLocalData.begin(); mapLocalData.end() != it; ++it)
		_log("%s - %s %s %s", it->first.c_str(), it->second.strName.c_str(), it->second.strPath.c_str(),
				it->second.strType.c_str());

}

void CAnalysisHandler::loadDictionary()
{

}

int CAnalysisHandler::evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch)
{
	return 0;
}

