/*
 * CJudgeStory.h
 *
 *  Created on: 2017年5月2日
 *      Author: Jugo
 */

#pragma once
#include "CSemantic.h"

class JSONObject;

class CJudgeStory: public CSemantic
{
public:
	explicit CJudgeStory();
	virtual ~CJudgeStory();
	int word(const char *szInput, JSONObject* jsonResp);
	int evaluate(const char *szWord);

private:
	void loadStoryDictionary();
};
