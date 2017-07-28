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

#define DISPLAY				"display"
#define ACTIVITY			"activity"
#define SHOW				"show"
#define ANIMATION			"animation"
#define TEXT				"text"

using namespace std;

extern map<string, JSONArray> mapStoryMood;

CResponsePacket::CResponsePacket() :
		jsonRoot(0)
{
	init();
//	jsonDisplay = new JSONObject;
//	jsonDisplay->create();
//	jsonDisplay->put("enable", 0);

//	jsonAyShow = new JSONArray;
//	jsonAyShow->create();
//
//	jsonAnimation = new JSONObject;
//	jsonAnimation->create();
//	jsonAnimation->put("type", 0);
//
//	jsonText = new JSONObject;
//	jsonText->create();
//	jsonText->put("type", 0);

//	jsonActivity = new JSONObject;
//	jsonActivity->create();
}

CResponsePacket::~CResponsePacket()
{
	jsonRoot->release();
//	jsonDisplay->release();
//	jsonAnimation->release();
//	jsonText->release();
//	jsonAyShow->release();
//	jsonActivity->release();

	delete jsonRoot;
//	delete jsonDisplay;
//	delete jsonAnimation;
//	delete jsonText;
//	delete jsonAyShow;
//	delete jsonActivity;
}

void CResponsePacket::init()
{
	if(jsonRoot)
	{
		jsonRoot->release();
		delete jsonRoot;
		jsonRoot = 0;
	}

	JSONObject jsonobject;
	jsonRoot = new JSONObject;
	jsonRoot->create();

	JSONObject jsonDisplay;
	JSONObject jsonActivity;
	JSONArray jsonArrShow;

	jsonRoot->put(DISPLAY, jsonDisplay);
	jsonRoot->put(ACTIVITY, jsonActivity);

	jsonDisplay = jsonRoot->getJsonObject(DISPLAY);
	jsonDisplay.put("enable", 0);
	jsonDisplay.put(SHOW, jsonArrShow);

	jsonActivity = jsonRoot->getJsonObject(ACTIVITY);
	jsonActivity.put("type", 0);
}

void CResponsePacket::format(int nType, JSONObject &jResp)
{
//	jResp.put("type", nType);

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
			//jsonAyShow->a = &mapStoryMood[trim(jsonRoot->getString("file"))];
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
//	if(jsonAyShow->size())
//	{
//		jsonDisplay->put("enable", 1);
//		jsonDisplay->put("show", *jsonAyShow);
//	}
//	jResp.put("display", *jsonDisplay);
//	jsonDisplay->release();
//	jsonAyShow->release();
//
//	jResp.put("activity", *jsonActivity);
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
	init();
}

CResponsePacket &CResponsePacket::setData(const char *szKey, JSONObject &jsonObj)
{
	jsonRoot->put(szKey, jsonObj.toJSON());
	return (*this);
}

CResponsePacket &CResponsePacket::setAnimation(const int nType, const int nDuration, const int nRepeat,
		const int nInterpolate)
{
//	jsonAnimation->put("type", nType);
//	jsonAnimation->put("duration", nDuration);
//	jsonAnimation->put("repeat", nRepeat);
//	jsonAnimation->put("interpolate", nInterpolate);
	return (*this);
}

CResponsePacket &CResponsePacket::setText(const int nType, const int nSize, const int nPosition, const char *szContain)
{
//	jsonText->put("type", nType);
//	jsonText->put("size", nSize);
//	jsonText->put("position", nPosition);
//	jsonText->put("contain", szContain);
	return (*this);
}

CResponsePacket &CResponsePacket::addShow(double fTime, const char *szHost, const char *szFile, const char *szColor,
		const char *szDesc)
{
	JSONObject jsonShow;
	jsonShow.create();
	jsonShow.put("time", fTime);
	jsonShow.put("host", szHost);
	jsonShow.put("file", szFile);
	jsonShow.put("color", szColor);
	jsonShow.put("description", szDesc);
//	jsonShow.put("animation", *jsonAnimation);
//	jsonShow.put("text", *jsonText);
//	jsonAyShow->add(jsonShow);
	jsonShow.release();
	return (*this);
}

CResponsePacket &CResponsePacket::addShow(double fTime, const char *szHost, const char *szFile, const char *szColor,
		const char *szDesc, JSONObject &jAnim, JSONObject &jText)
{
	JSONObject jsonShow;
	JSONObject jsonDisplay;
	JSONArray jsonArrShow;

	jsonDisplay = jsonRoot->getJsonObject(DISPLAY);
	jsonArrShow = jsonDisplay.getJsonArray(SHOW);

	jsonShow.create();
	jsonShow.put("time", fTime);
	jsonShow.put("host", szHost);
	jsonShow.put("file", szFile);
	jsonShow.put("color", szColor);
	jsonShow.put("description", szDesc);
	jsonShow.put(ANIMATION, jAnim);
	jsonShow.put(TEXT, jText);
	jsonArrShow.add(jsonShow);
//	jsonAyShow->add(jsonShow);
	jsonShow.release();
	return (*this);
}
