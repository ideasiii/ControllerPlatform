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

	if(string::npos != strWord.find("故事"))
	{
		JSONObject jsonStory;
		jsonStory.put("host", STORY_HOST);
		jsonStory.put("story", "三隻小豬.mp3");
		jsonStory.put("title", "三隻小豬");
		jsonResp->put("type", TYPE_RESP_STORY);
		jsonResp->put("story", jsonStory);
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

