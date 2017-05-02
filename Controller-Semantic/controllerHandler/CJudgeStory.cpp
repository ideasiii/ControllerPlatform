/*
 * CJudgeStory.cpp
 *
 *  Created on: 2017年5月2日
 *      Author: Jugo
 */

#include <map>
#include <string>
#include "CJudgeStory.h"
#include "JSONObject.h"
#include "config.h"
#include "dic_story.h"
#include "dic_semantic.h"
#include "common.h"

using namespace std;

CJudgeStory::CJudgeStory()
{

}

CJudgeStory::~CJudgeStory()
{

}

int CJudgeStory::word(const char *szInput, JSONObject* jsonResp)
{
	bool bMatch = false;
	JSONObject jsonStory;
	string strWord;

	strWord = szInput;
	if(strWord.empty())
		return FALSE;

	jsonStory.put("host", STORY_HOST);
	jsonResp->put("type", TYPE_RESP_STORY);

	for(map<string, string>::iterator iter = mapStory.begin(); mapStory.end() != iter; ++iter)
	{
		if(string::npos != strWord.find(iter->first))
		{
			bMatch = true;
			jsonStory.put("file", iter->second);
			jsonStory.put("story", iter->first);
			break;
		}
	}

	if(!bMatch)
	{
		jsonStory.put("file", "三隻小豬.mp3");
		jsonStory.put("story", "三隻小豬");
	}

	jsonResp->put("story", jsonStory);
	return TRUE;
}

int CJudgeStory::evaluate(const char *szWord)
{
	int nScore;
	string strWord;

	nScore = 0;
	strWord = szWord;

	if(strWord.empty())
		return 0;

	//======== 評估關鍵字 ==========//
	if(string::npos != strWord.find("故事") || string::npos != strWord.find("story"))
		++nScore;

	//======== 評估字典檔 ==========//
	for(map<string, string>::iterator iter = mapStory.begin(); mapStory.end() != iter; ++iter)
	{
		if(string::npos != strWord.find(iter->first))
		{
			++nScore;
			break;
		}
	}

	//======== 評估動詞 ===========//
	WORD_ATTR wordAttr;
	if(0 <= getVerb(strWord.c_str(), wordAttr))
	{
		if(VERB_LISTEN == wordAttr.nSubAttr)
		{
			++nScore;
			_log("[CJudgeStory] evaluate VERB: %s", wordAttr.strWord.c_str());
		}
	}

	return nScore;
}

