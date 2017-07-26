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

typedef struct _CONF
{
	std::string strPath;
	std::string strName;
	std::string strFileType;
	int nType;
} CONF;

struct LOCAL_DATA
{
	std::string strName;
	std::string strPath;
	std::string strType;
};

struct DICTIONARY
{
	std::string strName;
	std::string strPath;
	int nType;
};

union UDATA
{
	LOCAL_DATA localData;
	DICTIONARY dictionary;
	UDATA()
	{
	}
	~UDATA()
	{
	}
};

struct RESOURCE
{
	std::string strName;
	std::string strPath;
	std::string strType;
	UDATA udata;
};

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

private:
	bool mbValid;
	CONF conf;
	std::map<std::string, RESOURCE> mapData;
	std::set<std::string> setKeyWord;
	std::set<std::string> setDictionary;
};
