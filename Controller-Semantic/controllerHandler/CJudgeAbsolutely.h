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
	int word(const char *szInput, JSONObject* jsonResp);
	int evaluate(const char *szWord);

private:
	void loadAbsolutelyDictionary();
};
