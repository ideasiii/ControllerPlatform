/*
 * CResponsePacket.cpp
 *
 *  Created on: 2017年6月22日
 *      Author: Jugo
 */

#include "CResponsePacket.h"
#include "config.h"
#include "LogHandler.h"
#include "utility.h"
#include "JSONArray.h"
#include "JSONObject.h"
#include "common.h"
#include <map>

using namespace std;

extern map<string, JSONArray> mapStoryMood;

CResponsePacket::CResponsePacket()
{
	jsonRoot = new JSONObject;
	jsonRoot->create();
}

CResponsePacket::~CResponsePacket()
{
	jsonRoot->release();
	delete jsonRoot;
}

void CResponsePacket::format(int nType, JSONObject &jResp)
{

	jResp.put("type", nType);

	switch(nType)
	{
	case TYPE_RESP_UNKNOW:
		break;
	case TYPE_RESP_MUSIC_SPOTIFY:
		jResp.put("music", *jsonRoot);
		break;
	case TYPE_RESP_STORY:
		if(mapStoryMood.end() != mapStoryMood.find(trim(jsonRoot->getString("file"))))
		{
			jsonRoot->put("mood", mapStoryMood[trim(jsonRoot->getString("file"))]);
		}
		jResp.put("story", jsonRoot->toJSON());
		break;
	case TYPE_RESP_TTS:
		jResp.put("tts", *jsonRoot);
		break;
	case TYPE_RESP_MUSIC_LOCAL:
		break;
	default:
		jResp.put("type", TYPE_RESP_UNKNOW);
	}
	jsonRoot->release();
}

CResponsePacket &CResponsePacket::setData(const char *szKey, const char *szValue)
{
	jsonRoot->put(szKey, szValue);
	return (*this);
}

CResponsePacket &CResponsePacket::setData(const char *szKey, std::string strValue)
{
	jsonRoot->put(szKey, strValue);
	return (*this);
}

CResponsePacket &CResponsePacket::setData(const char *szKey, int nValue)
{
	jsonRoot->put(szKey, nValue);
	return (*this);
}

CResponsePacket &CResponsePacket::setData(const char *szKey, double fValue)
{
	jsonRoot->put(szKey, fValue);
	return (*this);
}

void CResponsePacket::clear()
{
	jsonRoot->create();
}

