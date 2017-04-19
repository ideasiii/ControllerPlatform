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
		jsonResp->put("type", TYPE_RESP_STORY);
		jsonResp->put("host", STORY_HOST);
		jsonResp->put("story", "三隻小豬.mp3");
		jsonResp->put("title", "三隻小豬");
	}
}

