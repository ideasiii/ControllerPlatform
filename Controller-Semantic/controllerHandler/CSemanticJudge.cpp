/*
 * CSemanticJudge.cpp
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#include <dic_music_mood.h>
#include <dic_semantic.h>
#include <dic_story.h>
#include <map>
#include <string>
#include "CSemanticJudge.h"
#include "JSONObject.h"
#include "config.h"
#include "common.h"
#include "Handler.h"
#include "CObject.h"
#include "CJudgeStory.h"

using namespace std;

CSemanticJudge::CSemanticJudge(CObject *object)
{
	mpController = object;
	mpJudgeStory = new CJudgeStory;
}

CSemanticJudge::~CSemanticJudge()
{
	delete mpJudgeStory;
}

int CSemanticJudge::word(const char *szInput, JSONObject *jsonResp)
{
	int nHigher;
	int nScore;
	int nIndex;
	int nSubject;
	string strWord;

	if(0 >= szInput)
	{
		jsonResp->put("type", 0);
		return TRUE;
	}

	nScore = 0;
	nHigher = TYPE_RESP_UNKNOW;
	strWord = szInput;

	//=============== Dummy ========================================//
	/**
	 *  情境1：故事
	 *  關鍵字："故事" + ("三隻小豬" || "小美人魚" || "睡美人" || "醜小鴨")
	 */
	nScore = mpJudgeStory->evaluate(strWord.c_str());
	_log("[CSemanticJudge] word - Judge Story Score: %d", nScore);

	mpJudgeStory->word(strWord.c_str(), jsonResp);
	/**
	 *  情境2：聽歌 From Spotify
	 *  關鍵字：("歌") + (??)
	 */
	if(string::npos != strWord.find("歌")) // spotify
	{
		JSONObject jsonSpotify;
		jsonSpotify.put("source", 2);
		jsonSpotify.put("album", "");
		jsonSpotify.put("artist", "");
		jsonSpotify.put("song", "");
		jsonSpotify.put("id", "spotify:track:2TpxZ7JUBn3uw46aR7qd6V");
		jsonResp->put("type", TYPE_RESP_MUSIC);
		jsonResp->put("music", jsonSpotify);
		return TRUE;
	}

	/**
	 *  情境3：聽有關情緒關鍵字的音樂
	 *  關鍵字：("音樂") + ("歡喜" || "憤怒" || "悲傷" || "驚恐" || "愛情")
	 */
	if(string::npos != strWord.find("音樂")) // Local
	{
		bool bMatch = false;
		JSONObject jsonMusic;
		jsonMusic.put("source", 1);
		jsonMusic.put("album", "");
		jsonMusic.put("artist", "");
		jsonMusic.put("song", "");
		jsonMusic.put("host", MUSIC_LOCAL_HOST);
		for(map<string, string>::iterator iter = mapMood.begin(); mapMood.end() != iter; ++iter)
		{
			if(string::npos != strWord.find(iter->first))
			{
				jsonMusic.put("file", iter->second);
				jsonMusic.put("song", iter->first);
				break;
			}
		}
		if(!bMatch)
		{
			jsonMusic.put("file", "love.mp3");
			jsonMusic.put("song", "愛情");
		}
		jsonResp->put("type", TYPE_RESP_MUSIC);
		jsonResp->put("music", jsonMusic);
		return TRUE;
	}

	return FALSE;
}
