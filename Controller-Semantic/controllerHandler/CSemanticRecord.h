/*
 * CSemanticRecord.h
 *
 *  Created on: 2017年5月2日
 *      Author: Jugo
 */

#pragma once

class CObject;

class JSONObject;
class CObject;

class CSemanticRecord
{
public:
	explicit CSemanticRecord(CObject *object);
	virtual ~CSemanticRecord();
	int word(const char *szInput, JSONObject* jsonResp);
};
