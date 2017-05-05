/*
 * CSemanticTalk.h
 *
 *  Created on: 2017年5月2日
 *      Author: Jugo
 */

#pragma once

class CObject;

class JSONObject;
class CObject;

class CSemanticTalk
{
public:
	explicit CSemanticTalk(CObject *object);
	virtual ~CSemanticTalk();
	int word(const char *szInput, JSONObject* jsonResp);
};
