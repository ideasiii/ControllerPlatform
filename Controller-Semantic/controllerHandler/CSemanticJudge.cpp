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
#include "dictionary.h"
#include "common.h"
#include "story.h"
#include "music.h"

using namespace std;

CSemanticJudge::CSemanticJudge()
{

}

CSemanticJudge::~CSemanticJudge()
{

}

void CSemanticJudge::word(const char *szInput, JSONObject *jsonResp)
{
	int nSubject;
	string strWord;

	if(0 >= szInput)
	{
		jsonResp->put("type", 0);
		return;
	}

	strWord = szInput;

	WORD_BODY wordBody;
	getAttribute(szInput, wordBody);

	for(WORD_BODY::iterator iter = wordBody.begin(); iter != wordBody.end(); ++iter)
	{
		_DBG("attr:%d index:%d subattr:%d word:%s", iter->second.nAttribute, iter->second.nIndex, iter->second.nSubAttr,
				iter->second.strWord.c_str());
	}

	//=============== Dummy ========================================//
	/**
	 *  情境1：故事
	 *  關鍵字："故事" + ("三隻小豬" || "小美人魚" || "睡美人" || "醜小鴨")
	 */
	if(string::npos != strWord.find("故事"))
	{
		JSONObject jsonStory;
		jsonStory.put("host", STORY_HOST);
		jsonResp->put("type", TYPE_RESP_STORY);
		jsonStory.put("story", "三隻小豬.mp3");
		jsonStory.put("title", "三隻小豬");
		for(map<string, string>::iterator iter = mapStory.begin(); mapStory.end() != iter; ++iter)
		{
			if(string::npos != strWord.find(iter->first))
			{
				jsonStory.put("file", iter->second);
				jsonStory.put("story", iter->first);
				break;
			}
		}
		jsonResp->put("story", jsonStory);
		jsonStory.release();
		return;
	}

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
		jsonSpotify.release();
		return;
	}

	/**
	 *  情境3：聽有關情緒關鍵字的音樂
	 *  關鍵字：("音樂") + ("歡喜" || "忿怒" || "哀傷" || "驚恐" || "愛情")
	 */
	if(string::npos != strWord.find("音樂")) // Local
	{
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
		jsonResp->put("type", TYPE_RESP_MUSIC);
		jsonResp->put("music", jsonMusic);
		jsonMusic.release();
		return;
	}
}

int CSemanticJudge::getSubject(const char *szWord)
{
	if(0 < szWord)
	{
		string strWord = szWord;
		map<string, int>::iterator it;

		for(it = mapSubject.begin(); it != mapSubject.end(); ++it)
		{
			if(string::npos != strWord.find(it->first))
				return it->second;
		}
	}

	return 0;
}

int CSemanticJudge::getAttribute(const char *szWord, WORD_BODY &wordBody)
{
	int nCount;
	int nNumber;
	int nIndex;
	string strWord = szWord;
	map<string, int>::iterator it;

	// 主詞
	nNumber = 0;
	nCount = 0;
	for(it = mapSubject.begin(); it != mapSubject.end(); ++it)
	{
		nIndex = strWord.find(it->first);
		if((int) string::npos != nIndex)
		{
			nNumber = nCount++;
			if(MAX_WORD_ATTR <= nNumber)
				break;
			WORD_ATTR wordAttr;
			wordAttr.nAttribute = SUBJECT;
			wordAttr.nIndex = nIndex;
			wordAttr.nSubAttr = it->second;
			wordAttr.strWord = it->first;
			wordBody[nNumber] = wordAttr;
		}
	}
	return 0;
}

