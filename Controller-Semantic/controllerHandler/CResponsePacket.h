/*
 * CResponsePacket.h
 *
 *  Created on: 2017年6月22日
 *      Author: Jugo
 */

#pragma once

#include <string>

class JSONObject;
class JSONArray;

class CResponsePacket
{
public:
	explicit CResponsePacket();
	virtual ~CResponsePacket();
	CResponsePacket &setData(const char *szKey, const char *szValue);
	CResponsePacket &setData(const char *szKey, std::string strValue);
	CResponsePacket &setData(const char *szKey, int nValue);
	CResponsePacket &setData(const char *szKey, double fValue);
	CResponsePacket &setData(const char *szKey, JSONObject &jsonObj);
	CResponsePacket &setAnimation(const int nType, const int nDuration, const int nRepeat, const int nInterpolate);
	CResponsePacket &setText(const int nType, const int nSize, const int nPosition, const char *szContain);
	CResponsePacket &addShow(double fTime, const char *szHost, const char *szFile, const char *szColor,
			const char *szDesc);
	CResponsePacket &addShow(double fTime, const char *szHost, const char *szFile, const char *szColor,
			const char *szDesc, JSONObject &jAnim, JSONObject &jText);
	void format(int nType, JSONObject &jResp);
	void clear();

private:
	JSONObject *jsonRoot;
	JSONObject *jsonDisplay;
	JSONArray *jsonAyShow;
	JSONObject *jsonAnimation;
	JSONObject *jsonText;
};
