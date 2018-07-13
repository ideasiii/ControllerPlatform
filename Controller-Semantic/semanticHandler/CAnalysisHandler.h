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

class CObject;
class CContentHandler;

enum enuWORD
{
	WORD_UNKNOW = 0, WORD_ERROR
};

class CAnalysisHandler: public CSemantic
{
	enum _CONF_TYPE
	{
		CONF_TYPE_UNDEFINE = -1, CONF_TYPE_LOCAL_FILE = 0, CONF_TYPE_DICTIONARY = 1
	};

	struct WORD
	{
		std::string WORD_UNKNOW;
		std::string WORD_ERROR;
	};

public:
	CAnalysisHandler(const char *szConf, CObject *object = 0);
	virtual ~CAnalysisHandler();
	int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);
	int activity(const char *szInput, JSONObject& jsonResp, std::map<std::string, std::string> &mapMatch);
	int service(const char *szInput, JSONObject& jsonResp, std::map<std::string, std::string> &mapMatch);
	std::string getName();
	std::string getWord(enuWORD ew);

private:
	bool loadConf(const char *szConf);
	void loadData();
	void loadLocalFile();
	void loadDictionary();
	void loadKeyWord(const char *szWord);
	void loadVerb(const char *szWord);
	void loadMatch(const char *szPath);
	std::string getDisplay(const char *szFile);
	void serviceSpotify(const char *szWord, const char *szArtist, JSONObject& jsonResp);
	void serviceWeather(const char *szWord, const char *szLocal, JSONObject& jsonResp);
	void serviceNews(const char *szWord, const char *szLocal, JSONObject& jsonResp);

private:
	std::map<std::string, RESOURCE> mapData;
	CONF conf;
	std::set<std::string> setKeyWord;
	std::set<std::string> setDictionary;
	VOCABULARY vocabulary;
	std::map<std::string, std::string> mapMatchWord;
	CContentHandler *contentHandler;

private:
	// Service Function Pointer
	typedef void (CAnalysisHandler::*MemFn)(const char *, const char *, JSONObject&);
	std::map<int, MemFn> mapFunc;
	CObject *m_pParent;
	WORD mWord;
};
