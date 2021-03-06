/*
 * CJudgeEducation.h
 *
 *  Created on: 2017年6月7日
 *      Author: root
 */

#pragma once

#include "CSemantic.h"

class CJudgeEducation: public CSemantic
{
public:
	CJudgeEducation();
	virtual ~CJudgeEducation();

protected:
	std::string toString();
	int word(const char *szInput, JSONObject& jsonResp, std::map<std::string, std::string> &mapMatch);
	int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);

private:
	void loadEducationDictionary();
};
