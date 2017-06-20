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
#include "CRankingHandler.cpp"
#include "CSemantic.h"

using namespace std;

CSemanticJudge::CSemanticJudge(CObject *object)
{
	mpController = object;
	mapSemanticObject[CONTENT_STORY] = new CJudgeStory;
	mapSemanticObject[CONTENT_MUSIC_SPOTIFY] = new CJudgeMusic;
	mapSemanticObject[CONTENT_ABSOLUTELY] = new CJudgeAbsolutely;
	mapSemanticObject[CONTENT_EDUCATION] = new CJudgeEducation;
	mapSemanticObject[CONTENT_SERVICE] = new CJudgeService;
}

CSemanticJudge::~CSemanticJudge()
{

	for(map<int, CSemantic*>::const_iterator it_map = mapSemanticObject.begin(); mapSemanticObject.end() != it_map;
			++it_map)
	{
		delete it_map->second;
	}
	mapSemanticObject.clear();
}

int CSemanticJudge::word(const char *szInput, JSONObject *jsonResp)
{
	int nTop;
	int nScore;
	int nIndex;
	int nSubject;
	int nValue;
	map<int, CSemantic*>::const_iterator iter;
	CRankingHandler<int, int> ranking;
	map<string, string> mapMatch;

	if(0 >= szInput)
	{
		jsonResp->put("type", 0);
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

	jsonResp->put("type", 3);
	jsonResp->put("tts", WORD_UNKNOW);

	return TRUE;
}
