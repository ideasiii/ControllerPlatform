/*
 * CResponsePacket.h
 *
 *  Created on: 2017年6月22日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include "JSONObject.h"

enum TYPE_RESP
{
	RESP_UNDEFINE = 0, RESP_LOCAL = 1, RESP_SPOTIFY = 2, RESP_TTS = 3
};

//class JSONObject;
//class JSONArray;

class CResponsePacket
{
public:
	explicit CResponsePacket();
	virtual ~CResponsePacket();
	template<typename T> CResponsePacket &setActivity(const char *szKey, T tValue)
	{
		jsonActivity->put(szKey, tValue);
		return (*this);
	}
	;
	void format(JSONObject &jResp);
	void clear();

private:
	void init();

private:
	//JSONObject *jsonRoot;
	JSONObject *jsonDisplay;
	JSONObject *jsonActivity;
	//JSONArray *jsonAyShow;
	//JSONObject *jsonAnimation;
	//JSONObject *jsonText;

};
