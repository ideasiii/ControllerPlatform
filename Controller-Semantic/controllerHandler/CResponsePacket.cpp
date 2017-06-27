/*
 * CResponsePacket.cpp
 *
 *  Created on: 2017年6月22日
 *      Author: Jugo
 */

#include "CResponsePacket.h"
#include "config.h"
#include "LogHandler.h"
#include "dictionary.h"

CResponsePacket::CResponsePacket()
{
	jsonRoot.create();
}

CResponsePacket::~CResponsePacket()
{
	jsonRoot.release();
}

void CResponsePacket::format(int nType, JSONObject &jResp)
{
	extern map<string, string> mapStory;
	extern map<string, JSONArray> mapStoryMood;

	jResp.put("type", nType);

	switch(nType)
	{
	case TYPE_RESP_UNKNOW:
		break;
	case TYPE_RESP_MUSIC_SPOTIFY:
		jResp.put("music", jsonRoot);
		break;
	case TYPE_RESP_STORY:
	{
		_log("[CResponsePacket] format Load Story: %d", mapStory.size());
		_log("[CResponsePacket] format Story Mood: %d", mapStoryMood.size());
		for(map<string, JSONArray>::iterator it = mapStoryMood.begin(); mapStoryMood.end() != it; ++it)
			_log("%s - %s", it->first.c_str(), it->second.toString().c_str());
		if(mapStoryMood.end() != mapStoryMood.find(jsonRoot.getString("story")))
		{
			jsonRoot.put("mood", mapStoryMood[jsonRoot.getString("story")]);
		}
		jResp.put("story", jsonRoot);
	}
		break;
	case TYPE_RESP_TTS:
		jResp.put("tts", jsonRoot);
		break;
	case TYPE_RESP_MUSIC_LOCAL:
		break;
	default:
		jResp.put("type", TYPE_RESP_UNKNOW);
	}
	jsonRoot.release();
}

CResponsePacket &CResponsePacket::setData(const char *szKey, const char *szValue)
{
	jsonRoot.put(szKey, szValue);
	return (*this);
}

CResponsePacket &CResponsePacket::setData(const char *szKey, std::string strValue)
{
	jsonRoot.put(szKey, strValue);
	return (*this);
}

CResponsePacket &CResponsePacket::setData(const char *szKey, int nValue)
{
	jsonRoot.put(szKey, nValue);
	return (*this);
}

CResponsePacket &CResponsePacket::setData(const char *szKey, double fValue)
{
	jsonRoot.put(szKey, fValue);
	return (*this);
}

void CResponsePacket::clear()
{
	jsonRoot.create();
}

