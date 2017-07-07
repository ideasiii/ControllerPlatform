/*
 * CJudgeTranslate.h
 *
 *  Created on: 2017年7月3日
 *      Author: jugo
 */

#pragma once

#include "CSemantic.h"

class JSONObject;

class CJudgeTranslate: public CSemantic
{
public:
	explicit CJudgeTranslate();
	virtual ~CJudgeTranslate();

protected:
	std::string toString();
	int word(const char *szInput, JSONObject& jsonResp, std::map<std::string, std::string> &mapMatch);
	int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);

};
