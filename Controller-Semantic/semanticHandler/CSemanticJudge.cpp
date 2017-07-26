/*
 * CSemanticJudge.cpp
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#include <map>
#include <string>
#include "CSemanticJudge.h"
#include "JSONObject.h"
#include "config.h"
#include "common.h"
#include "Handler.h"
#include "CObject.h"
#include "CJudgeAbsolutely.h"
#include "CJudgeStory.h"
#include "CJudgeMusic.h"
#include "CJudgeEducation.h"
#include "CJudgeService.h"
#include "CJudgeTranslate.h"
#include "CRankingHandler.cpp"
#include "CSemantic.h"
#include "CResponsePacket.h"
#include "CAnalysisHandler.h"
#include "CConfig.h"
#include "utility.h"

using namespace std;

CSemanticJudge::CSemanticJudge(CObject *object)
{
	mpController = object;
	mapSemanticObject[CONTENT_STORY] = new CJudgeStory;
	mapSemanticObject[CONTENT_MUSIC_SPOTIFY] = new CJudgeMusic;
	mapSemanticObject[CONTENT_ABSOLUTELY] = new CJudgeAbsolutely;
	mapSemanticObject[CONTENT_EDUCATION] = new CJudgeEducation;
	mapSemanticObject[CONTENT_SERVICE] = new CJudgeService;
	mapSemanticObject[CONTENT_TRANSLATE] = new CJudgeTranslate;

	loadConfig();
}

CSemanticJudge::~CSemanticJudge()
{

	for(map<int, CSemantic*>::const_iterator it_map = mapSemanticObject.begin(); mapSemanticObject.end() != it_map;
			++it_map)
	{
		delete it_map->second;
	}
	mapSemanticObject.clear();

	for(unsigned int i = 1; i <= mapAnalysis.size(); ++i)
	{
		delete mapAnalysis[i];
	}
	mapAnalysis.clear();
}

void CSemanticJudge::loadConfig()
{
	int nValue;
	string strValue;
	CConfig *config;

	config = new CConfig();
	if(config->loadConfig(getConfigFile()))
	{
		strValue = config->getValue("ANALYSIS", "total");
		convertFromString(nValue, strValue);
		_log("[CSemanticJudge] CSemanticJudge get analysis total: %d", nValue);
		for(int i = 1; i <= nValue; ++i)
		{
			strValue = config->getValue("ANALYSIS", ConvertToString(i));
			_log("[CSemanticJudge] CSemanticJudge Get conf: %d conf file: %s", i, strValue.c_str());
			mapAnalysis[i] = new CAnalysisHandler(strValue.c_str());
		}
	}
	else
	{
		_log("[CSemanticJudge] CSemanticJudge Load Configure Fail, File: %s", getConfigFile().c_str());
	}

	delete config;
}

int CSemanticJudge::word(const char *szInput, JSONObject &jsonResp)
{
	int nTop;
	int nScore;
	int nIndex;
	int nSubject;
	int nValue;
	map<int, CSemantic*>::const_iterator iter;
	CRankingHandler<int, int> ranking;
	map<string, string> mapMatch;
	CResponsePacket respPacket;

	if(0 >= szInput)
	{
		respPacket.setData("lang", "zh").setData("content", WORD_UNKNOW).format(TYPE_RESP_TTS, jsonResp);
		return TRUE;
	}

	_log("[CSemanticJudge] word input: %s", szInput);

	nScore = 0;

	for(iter = mapSemanticObject.begin(); mapSemanticObject.end() != iter; ++iter)
	{
		nScore = iter->second->_evaluate(szInput, mapMatch);
		ranking.add(iter->first, nScore);
		_log("[CSemanticJudge] word - %s Get Score: %d", iter->second->_toString().c_str(), nScore);
	}

	for(unsigned int i = 1; i <= mapAnalysis.size(); ++i)
	{
		nScore = mapAnalysis[i]->evaluate(szInput, mapMatch);
		_log("[CSemanticJudge] word Analysis - %s Get Score: %d", mapAnalysis[i]->getName().c_str(), nScore);
	}
//============== 積分比較 ================//
	nValue = ranking.topValue();
	if(0 < nValue)
	{
		nTop = ranking.topValueKey();
		_log("[CSemanticJudge] word Top Key is %d", nTop);
		if(mapSemanticObject.end() != mapSemanticObject.find(nTop))
		{
			mapSemanticObject[nTop]->_word(szInput, jsonResp, mapMatch);
			return TRUE;
		}
		_log("[CSemanticJudge] word: No Object to Access this work");
	}

	respPacket.setData("lang", "zh").setData("content", WORD_UNKNOW).format(TYPE_RESP_TTS, jsonResp);

	return TRUE;
}
