/*
 * CSemanticJudge.cpp
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#include "CSemanticJudge.h"
#include "JSONObject.h"
#include "config.h"

using namespace std;

CSemanticJudge::CSemanticJudge()
{

}

CSemanticJudge::~CSemanticJudge()
{

}

void CSemanticJudge::word(const char *szInput, JSONObject *jsonResp)
{
	string strWord;

	if(0 >= szInput)
	{
		jsonResp->put("type", 0);
		return;
	}

	strWord = szInput;

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

