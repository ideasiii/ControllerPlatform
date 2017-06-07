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
	int word(const char *szInput, JSONObject* jsonResp);
	int evaluate(const char *szWord);

private:
	void loadEducationDictionary();
};
