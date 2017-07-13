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

	jsonDisplay = new JSONObject;
	jsonDisplay->create();
	jsonDisplay->put("enable", 0);

	jsonAyShow = new JSONArray;
	jsonAyShow->create();

	jsonAnimation = new JSONObject;
	jsonAnimation->create();
	jsonAnimation->put("type", 0);
}

CResponsePacket::~CResponsePacket()
{
	jsonRoot->release();
	jsonDisplay->release();
	jsonAnimation->release();
	jsonAyShow->release();
	delete jsonRoot;
	delete jsonDisplay;
	delete jsonAnimation;
	delete jsonAyShow;
}

void CResponsePacket::format(int nType, JSONObject &jResp)
{
	jResp.put("type", nType);

	switch(nType)
	{
	case TYPE_RESP_UNKNOW:
		break;
	case TYPE_RESP_MUSIC_SPOTIFY:
		jResp.put("music", jsonRoot->toJSON());
		break;
	case TYPE_RESP_STORY:
		if(mapStoryMood.end() != mapStoryMood.find(trim(jsonRoot->getString("file"))))
		{
			jsonRoot->put("mood", mapStoryMood[trim(jsonRoot->getString("file"))]);
		}
		jResp.put("story", jsonRoot->toJSON());
		break;
	case TYPE_RESP_TTS:
		jResp.put("tts", jsonRoot->toJSON());
		break;
	case TYPE_RESP_MUSIC_LOCAL:
		break;
	default:
		jResp.put("type", TYPE_RESP_UNKNOW);
	}
	jsonRoot->release();

	jsonDisplay->put("show", jsonAyShow->toJSON());
	jResp.put("display", jsonDisplay->toJSON());
	jsonDisplay->release();
	jsonAyShow->release();
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

CResponsePacket &CResponsePacket::setData(const char *szKey, JSONObject &jsonObj)
{
	jsonRoot->put(szKey, jsonObj.toJSON());
	return (*this);
}

CResponsePacket &CResponsePacket::addShow(double fTime, const char *szHost, const char *szFile, const char *szColor,
		const char *szDesc, JSONObject &jsonAnim)
{
	JSONObject jsonShow;
	jsonShow.create();
	jsonShow.put("time", fTime);
	jsonShow.put("host", szHost);
	jsonShow.put("file", szFile);
	jsonShow.put("color", szColor);
	jsonShow.put("description", szDesc);
	jsonShow.put("animation", jsonAnim);
	jsonAyShow->add(jsonShow);
	jsonShow.release();
	return (*this);
}
