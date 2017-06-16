/*
 * CJudgeNews.h
 *
 *  Created on: 2017年6月16日
 *      Author: Jugo
 */

#pragma once

#include "CSemantic.h"

class CJudgeNews: public CSemantic
{
public:
	CJudgeNews();
	virtual ~CJudgeNews();

protected:
	std::string toString();
	int word(const char *szInput, JSONObject* jsonResp, std::map<std::string, std::string> &mapMatch);
	int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);

private:
	void loadNewsDictionary();
};
