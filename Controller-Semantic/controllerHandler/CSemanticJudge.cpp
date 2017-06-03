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
#include "CJudgeStory.h"
#include "CJudgeMusic.h"
#include "CRankingHandler.cpp"

using namespace std;

CSemanticJudge::CSemanticJudge(CObject *object)
{
	mpController = object;
	mpJudgeStory = new CJudgeStory;
	mpJudgeMusic = new CJudgeMusic;
}

CSemanticJudge::~CSemanticJudge()
{
	delete mpJudgeStory;
	delete mpJudgeMusic;
}

int CSemanticJudge::word(const char *szInput, JSONObject *jsonResp)
{
	int nTop;
	int nScore;
	int nIndex;
	int nSubject;
//	string strWord;
	CRankingHandler<int, int> ranking;

	if(0 >= szInput)
	{
		jsonResp->put("type", 0);
		return TRUE;
	}

	_log("[CSemanticJudge] word input: %s", szInput);

	nScore = 0;
//	strWord = szInput;

//============== 故事 ================//
	nScore = mpJudgeStory->evaluate(szInput);
	ranking.add(CONTENT_STORY, nScore);
	_log("[CSemanticJudge] word - Judge Story Score: %d", nScore);

//============== Spotify ================//
	nScore = mpJudgeMusic->evaluate(szInput);
	ranking.add(CONTENT_MUSIC_SPOTIFY, nScore);
	_log("[CSemanticJudge] word - Judge Music Score: %d", nScore);

//============== 積分比較 ================//
	nTop = ranking.topValueKey();
	_log("[CSemanticJudge] word Top Key is %d", nTop);

	switch(nTop)
	{
	case CONTENT_STORY:
		mpJudgeStory->word(szInput, jsonResp);
		break;
	case CONTENT_MUSIC_SPOTIFY:
		mpJudgeMusic->word(szInput, jsonResp);
		break;
	}

	/**
	 *  情境3：聽有關情緒關鍵字的音樂
	 *  關鍵字：("音樂") + ("歡喜" || "憤怒" || "悲傷" || "驚恐" || "愛情")
	 */
//	if(string::npos != strWord.find("音樂")) // Local
//	{
//		bool bMatch = false;
//		JSONObject jsonMusic;
//		jsonMusic.put("source", 1);
//		jsonMusic.put("album", "");
//		jsonMusic.put("artist", "");
//		jsonMusic.put("song", "");
//		jsonMusic.put("host", MUSIC_LOCAL_HOST);
//		for(map<string, string>::iterator iter = mapMood.begin(); mapMood.end() != iter; ++iter)
//		{
//			if(string::npos != strWord.find(iter->first))
//			{
//				jsonMusic.put("file", iter->second);
//				jsonMusic.put("song", iter->first);
//				break;
//			}
//		}
//		if(!bMatch)
//		{
//			jsonMusic.put("file", "love.mp3");
//			jsonMusic.put("song", "愛情");
//		}
//		jsonResp->put("type", TYPE_RESP_MUSIC);
//		jsonResp->put("music", jsonMusic);
//		return TRUE;
//	}
	return TRUE;
}
