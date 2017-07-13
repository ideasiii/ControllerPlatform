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
	CResponsePacket &addShow(double fTime, const char *szHost, const char *szFile, const char *szColor,
			const char *szDesc, JSONObject &jsonAnim);
	void format(int nType, JSONObject &jResp);
	void clear();

private:
	JSONObject *jsonRoot;
	JSONObject *jsonDisplay;
	JSONArray *jsonAyShow;
	JSONObject *jsonAnimation;
};
