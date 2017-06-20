/*
 * CJudgeService.cpp
 *
 *  Created on: 2017年6月9日
 *      Author: Jugo
 */

#include <algorithm>
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
#include "CTranslate.h"

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

int CJudgeService::word(const char *szInput, JSONObject* jsonResp, map<string, string> &mapMatch)
{
	int nService;
	string strWord;
	string strTTS;
	string strLocation;
	string strTranslate;
	map<string, int>::const_iterator it_map;
	int nYear, nMonth, nDay, nHour, nMinute, nSecond;

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
		currentDateTimeNum(nYear, nMonth, nDay, nHour, nMinute, nSecond);
		strTTS = format("現在時刻，%d點，%d分，%d秒", nHour, nMinute, nSecond);
		break;
	case SERVICE_WEATHER:
		strLocation = trim(weather.strLocation);
		getLocation(szInput, weather);
		if(60 < (nowSecond() - weather.lnToday) || trim(weather.strLocation).compare(strLocation))
		{
			getWeather(weather.strLocation.c_str(), weather);
			_log("[CJudgeService] word Update Weather: %s", weather.strLocation.c_str());
		}

		strTTS = format("現在%s天氣，%s;氣溫%.02f度;溼度%.02f度", weather.strLocation.c_str(), weather.strWeather.c_str(),
				weather.fTemperature, weather.fHumidity);
		break;
	case SERVICE_TRANSLATE:
		getTranslate(szInput, strTranslate);
		strTTS = strTranslate;
		break;
	}

	jsonResp->put("tts", strTTS);
	return 0;
}

int CJudgeService::evaluate(const char *szWord, map<string, string> &mapMatch)
{
	int nScore;
	string strWord;
	string strLocation;
	map<string, int>::const_iterator it_map;
	set<string>::const_iterator it_set;

	nScore = 0;
	strWord = szWord;

	if(strWord.empty())
		return 0;

	//======== 評估相同字 ==========//
	for(it_map = mapService.begin(); mapService.end() != it_map; ++it_map)
	{
		if(!trim(it_map->first).compare(trim(strWord)))
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
			if(SERVICE_WEATHER == it_map->second)
			{
				for(it_set = setLocation.begin(); setLocation.end() != it_set; ++it_set)
				{
					strLocation = trim(*it_set);
					transform(strLocation.begin(), strLocation.end(), strLocation.begin(), ::tolower);
					if(string::npos != strWord.find(strLocation))
					{
						++nScore;
						break;
					}
				}
			}
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

	setData.clear();
	fh.readAllLine("dictionary/location.txt", setLocation);
	_log("[CJudgeService] loadServiceDictionary Total Service Location Count: %d", setLocation.size());
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

void CJudgeService::getLocation(const char *szWord, WEATHER &weather)
{
	set<string>::const_iterator it_set;
	string strWord;
	string strLocation;

	if(!szWord)
		return;

	strWord = trim(szWord);
	transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);

	for(it_set = setLocation.begin(); setLocation.end() != it_set; ++it_set)
	{
		strLocation = trim(*it_set);
		transform(strLocation.begin(), strLocation.end(), strLocation.begin(), ::tolower);
		if(string::npos != strWord.find(strLocation))
		{
			weather.strLocation = strLocation;
			return;
		}
	}

	weather.strLocation = "台北";
}

void CJudgeService::getTranslate(const char *szWord, std::string &strResult)
{
	CTranslate translate;
	RESULT result;
	map<string, int>::const_iterator it_map;

	if(!szWord)
		return;

	//翻譯,3
	//中翻英,3

	for(it_map = mapService.begin(); mapService.end() != it_map; ++it_map)
	{
		if(string::npos != strWord.find(it_map->first))
		{
			nService = it_map->second;
			break;
		}
	}

	translate.translate(en, szWord, result);
	strResult = result.strResult;
}

