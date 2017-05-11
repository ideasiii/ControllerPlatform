/*
 * CJudgeMusic.h
 *
 *  Created on: 2017年5月11日
 *      Author: Jugo
 */

#pragma once

#include "CSemantic.h"

class JSONObject;

class CJudgeMusic: public CSemantic
{
public:
	explicit CJudgeMusic();
	virtual ~CJudgeMusic();
	int word(const char *szInput, JSONObject* jsonResp);
	int evaluate(const char *szWord);
};
