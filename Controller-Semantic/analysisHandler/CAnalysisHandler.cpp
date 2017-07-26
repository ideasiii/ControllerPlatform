/*
 * CAnalysisHandler.cpp
 *
 *  Created on: 2017年7月18日
 *      Author: Jugo
 */

#include <string>
#include "CAnalysisHandler.h"
#include "LogHandler.h"
#include "CConfig.h"
#include "CFileHandler.h"
#include "utility.h"
#include "config.h"

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
	mapData.clear();
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
				conf.strPath = config->getValue("FILE", "path");
				conf.strFileType = config->getValue("FILE", "type");
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

			loadKeyWord(config->getValue("KEY", "word").c_str());
		}
		delete config;
	}

	return bResult;
}

void CAnalysisHandler::loadKeyWord(const char *szWord)
{
	vector<string> vData;

	if(!szWord)
		return;

	setKeyWord.clear();
	spliteData(const_cast<char*>(szWord), ",", setKeyWord);

	for(set<string>::const_iterator it = setKeyWord.begin(); setKeyWord.end() != it; ++it)
	{
		_log("[CAnalysisHandler] loadKeyWord key word: %s", it->c_str());
	}
}

string CAnalysisHandler::getName()
{
	return conf.strName;
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
	string strFileName;
	CFileHandler fh;
	set<string> setData;
	set<string>::const_iterator iter_set;

	fh.readPath(conf.strPath.c_str(), setData);
	setDictionary.clear();
	for(iter_set = setData.begin(); setData.end() != iter_set; ++iter_set)
	{
		nIndex = iter_set->rfind(".");
		if((int) string::npos != nIndex)
		{
			if(!iter_set->substr(nIndex + 1).compare(conf.strFileType))
			{
				strFileName = iter_set->substr(0, nIndex);
				setDictionary.insert(trim(strFileName));
				mapData[strFileName].udata.localData.strName = strFileName;
				mapData[strFileName].udata.localData.strPath = conf.strPath + (*iter_set);
				mapData[strFileName].udata.localData.strType = conf.strFileType;
			}
		}
	}

	for(map<string, RESOURCE>::const_iterator it = mapData.begin(); mapData.end() != it; ++it)
		_log("%s - %s %s %s", it->first.c_str(), it->second.udata.localData.strName.c_str(),
				it->second.udata.localData.strPath.c_str(), it->second.udata.localData.strType.c_str());

}

void CAnalysisHandler::loadDictionary()
{

}

int CAnalysisHandler::evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch)
{
	int nScore;
	string strWord;
	string strValue;
	set<string>::const_iterator it_set;

	strWord = szWord;
	if(strWord.empty())
		return 0;

	nScore = 0;

	//======== 評估關鍵字 ==========//
	for(it_set = setKeyWord.begin(); setKeyWord.end() != it_set; ++it_set)
	{
		if(string::npos != strWord.find(*it_set))
		{
			++nScore;
			_log("[CAnalysisHandler] evaluate find key word: %s", it_set->c_str());
		}
	}

	//======== 評估字典檔 ==========//
	strValue.clear();
	for(it_set = setDictionary.begin(); setDictionary.end() != it_set; ++it_set)
	{
		if(string::npos != strWord.find(*it_set))
		{
			if(strValue.empty() || strValue.length() < it_set->length())
			{
				strValue = *it_set;
			}
		}
	}
	if(!strValue.empty())
	{
		mapMatch["dictionary"] = strValue;
		_log("[CAnalysisHandler] evaluate find dictionary word: %s", mapMatch["dictionary"].c_str());
		++nScore;
	}

	//======== 評估動詞 ===========//
//	WORD_ATTR wordAttr;
//	if(0 <= getVerb(strWord.c_str(), wordAttr))
//	{
//		if(VERB_LISTEN == wordAttr.nSubAttr)
//		{
//			++nScore;
//			_log("[CJudgeStory] evaluate VERB: %s", wordAttr.strWord.c_str());
//		}
//	}

	return nScore;
}

int CAnalysisHandler::activity(const char *szInput, JSONObject& jsonResp, std::map<std::string, std::string> &mapMatch)
{

}

