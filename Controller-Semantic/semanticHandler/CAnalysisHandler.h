/*
 * CAnalysisHandler.h
 *
 *  Created on: 2017年7月18日
 *      Author: Jugo
 */

#pragma once

#include <set>
#include <map>
#include <memory.h>
#include "CSemantic.h"
#include "dataStruct.h"

class CAnalysisHandler: public CSemantic
{
	enum _CONF_TYPE
	{
		CONF_TYPE_LOCAL_FILE = 0, CONF_TYPE_DICTIONARY
	};

public:
	CAnalysisHandler(const char *szConf);
	virtual ~CAnalysisHandler();
	int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);
	int activity(const char *szInput, JSONObject& jsonResp, std::map<std::string, std::string> &mapMatch);
	std::string getName();

private:
	bool loadConf(const char *szConf);
	void loadData();
	void loadLocalFile();
	void loadDictionary();
	void loadKeyWord(const char *szWord);
	void loadVerb(const char *szWord);
	void loadMatch(const char *szPath);

private:
	bool mbValid;
	std::map<std::string, RESOURCE> mapData;
	CONF conf;
	std::set<std::string> setKeyWord;
	std::set<std::string> setDictionary;
	VOCABULARY vocabulary;
	std::map<std::string, std::string> mapMatchWord;
};
