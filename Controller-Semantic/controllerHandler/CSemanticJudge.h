/*
 * CSemanticJudge.h
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#pragma once

class JSONObject;

class CSemanticJudge
{
public:
	explicit CSemanticJudge();
	virtual ~CSemanticJudge();
	void word(const char *szInput, JSONObject* jsonResp);
};
