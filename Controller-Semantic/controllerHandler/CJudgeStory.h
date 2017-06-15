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

protected:
	std::string toString();
	int word(const char *szInput, JSONObject* jsonResp, std::map<std::string, std::string> &mapMatch);
	int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);

private:
	void loadStoryDictionary();
};
