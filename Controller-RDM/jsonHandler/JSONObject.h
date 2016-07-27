/*
 * JSONObject.h
 *
 *  Created on: 2016年7月20日
 *      Author: Jugo
 */

#pragma once
#include <string>

class cJSON;
class JSONArray;

using namespace std;

class JSONObject
{
public:
	explicit JSONObject();
	explicit JSONObject(string strSource);
	explicit JSONObject(cJSON *pcJSON);
	bool getBoolean(string name);
	bool getBoolean(string name, bool defaultValue);
	int getInt(string name);
	int getInt(string name, int defaultValue);
	cJSON *getJsonArray(string strName);
	cJSON *getJsonObject(string strName);
	string getString(string name);
	string getString(string name, string defaultValue);
	bool isNull(string name);

	virtual ~JSONObject();
	bool isValid();
	string toString();
	void put(string strKey, string strValue);
	void put(string strKey, const char* szrValue);
	void put(string strKey, int nValue);
	void put(string strKey, bool bValue);
	void put(string strKey, double dValue);
	void put(string strKey, JSONObject &jsonObject);
	void put(string strKey, JSONArray &jsonArray);
	cJSON * getcJSON();

	void release();

private:
	cJSON *cjsonObj;
};
