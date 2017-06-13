/*
 * CJudgeService.cpp
 *
 *  Created on: 2017年6月9日
 *      Author: Jugo
 */

#include <set>
#include <string>
#include <map>
#include "CJudgeService.h"
#include "CFileHandler.h"
#include "LogHandler.h"
#include "utility.h"
#include "dictionary.h"
#include "config.h"
#include "common.h"
#include "JSONObject.h"

using namespace std;

CJudgeService::CJudgeService()
{
	loadServiceDictionary();
	weather.clear();
}

CJudgeService::~CJudgeService()
{

}

string CJudgeService::toString()
{
	return "CJudgeService";
}

int CJudgeService::word(const char *szInput, JSONObject* jsonResp)
{
	int nService;
	string strWord;
	string strTTS;
	map<string, int>::const_iterator it_map;

	strWord = szInput;
	if(strWord.empty())
		return FALSE;

	jsonResp->put("type", TYPE_RESP_TTS);
	for(it_map = mapService.begin(); mapService.end() != it_map; ++it_map)
	{
		if(string::npos != strWord.find(it_map->first))
		{
			nService = it_map->second;
			break;
		}
	}

	switch(nService)
	{
	case SERVICE_CLOCK:
		break;
	case SERVICE_WEATHER:
		getWeather("台北", weather);
		_log("############# %d", weather.lnToday);
		strTTS = format("現在天氣%s，氣溫%.02f度，溼度%.02f度", weather.strWeather.c_str(), weather.fTemperature,
				weather.fHumidity);
		break;
	}

	jsonResp->put("tts", strTTS);
	return 0;
}

int CJudgeService::evaluate(const char *szWord)
{
	int nScore;
	string strWord;
	map<string, int>::const_iterator it_map;

	nScore = 0;
	strWord = szWord;

	if(strWord.empty())
		return 0;

	//======== 評估相同字 ==========//
	for(it_map = mapService.begin(); mapService.end() != it_map; ++it_map)
	{
		if(!it_map->first.compare(strWord))
		{
			return 100;
		}
	}

	//======== 評估字典檔 ==========//
	for(it_map = mapService.begin(); mapService.end() != it_map; ++it_map)
	{
		if(string::npos != strWord.find(it_map->first))
		{
			++nScore;
			break;
		}
	}

	return nScore;
}

void CJudgeService::loadServiceDictionary()
{
	int nIndex;
	int nService;
	string strKey;
	string strValue;
	CFileHandler fh;
	set<string> setData;
	set<string>::const_iterator iter;

	fh.readAllLine("dictionary/match_service.txt", setData);
	for(iter = setData.begin(); setData.end() != iter; ++iter)
	{
		if(!iter->empty() && trim(*iter).length())
		{
			nIndex = iter->find(",");
			strKey = iter->substr(0, nIndex);
			strValue = iter->substr(nIndex + 1);
			convertFromString(nService, strValue);
			mapService[strKey] = nService;
		}
	}
	_log("[CJudgeService] loadServiceDictionary Total Service Match Count: %d", mapService.size());
}

void CJudgeService::getClock(string &strClock)
{
	string strNow;

}

void CJudgeService::getWeather(const char *szLocation, WEATHER &weather)
{
	CWeather wt;

	if(!szLocation)
		return;

	wt.getWeather(szLocation, weather);
}

