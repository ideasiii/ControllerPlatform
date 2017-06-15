/*
 * CJudgeAbsolutely.h
 *
 *  Created on: 2017年6月6日
 *      Author: root
 */

#pragma once

#include "CSemantic.h"

class CJudgeAbsolutely: public CSemantic
{
public:
	CJudgeAbsolutely();
	virtual ~CJudgeAbsolutely();

protected:
	std::string toString();
	int word(const char *szInput, JSONObject* jsonResp, std::map<std::string, std::string> &mapMatch);
	int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);

private:
	void loadAbsolutelyDictionary();
};
