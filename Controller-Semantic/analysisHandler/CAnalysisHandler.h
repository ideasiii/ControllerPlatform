/*
 * CAnalysisHandler.h
 *
 *  Created on: 2017年7月18日
 *      Author: Jugo
 */

#pragma once

#include <map>
#include <memory.h>
#include "CSemantic.h"

typedef struct _CONF_DICTIONARY
{
	std::string strPath;
} CONF_DICTIONARY;

typedef struct _CONF_FILE
{
	std::string strPath;
	std::string strType;
} CONF_FILE;

union U
{
	_CONF_FILE conf_file;
	_CONF_DICTIONARY conf_dictionary;
	U()
	{
		memset(this, 0, sizeof(U));
	}
	;
	~U()
	{
	}
	;
};

typedef struct _CONFIG
{
	std::string strName;
	int nType;
	U uConf;
} CONF;

typedef struct _LOCAL_DATA
{
	std::string strName;
	std::string strPath;
	std::string strType;
	void clear()
	{
		strName.clear();
		strPath.clear();
		strType.clear();
	}
} LOCAL_DATA;

typedef struct _DICTIONARY_DATA
{

} DICTIONARY_DATA;

union U_DATA
{
	LOCAL_DATA localData;
	DICTIONARY_DATA dictionaryData;
	U_DATA()
	{
		memset(this, 0, sizeof(U));
	}

	~U_DATA()
	{
	}
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

private:
	bool loadConf(const char *szConf);
	void loadData();
	void loadLocalFile();
	void loadDictionary();

private:
	bool mbValid;
	CONF conf;
	std::map<std::string, LOCAL_DATA> mapLocalData;
};
