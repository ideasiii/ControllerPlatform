/*
 * CResponsePacket.h
 *
 *  Created on: 2017年6月22日
 *      Author: Jugo
 */

#pragma once

#include "JSONObject.h"

enum TYPE_RESP
{
	RESP_UNDEFINE = 0, RESP_LOCAL = 1, RESP_SPOTIFY = 2, RESP_TTS = 3
};

class CResponsePacket
{
public:
	explicit CResponsePacket();
	virtual ~CResponsePacket();
	void format(JSONObject &jResp);
	void clear();
	template<typename T> CResponsePacket &setActivity(const char *szKey, T tValue)
	{
		jsonActivity->put(szKey, tValue);
		return (*this);
	}
	;

	CResponsePacket &setDisplay(const char *szDisplayJSON);

private:
	void init();

private:
	JSONObject *jsonDisplay;
	JSONObject *jsonActivity;
};
