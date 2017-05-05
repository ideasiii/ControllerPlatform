/*
 * CSemanticControl.h
 *
 *  Created on: 2017年5月2日
 *      Author: Jugo
 */

#pragma once

class CObject;

class JSONObject;
class CObject;

class CSemanticControl
{
public:
	explicit CSemanticControl(CObject *object);
	virtual ~CSemanticControl();
	int word(const char *szInput, JSONObject* jsonResp);
};
