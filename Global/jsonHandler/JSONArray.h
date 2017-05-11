/*
 * JSONArray.h
 *
 *  Created on: 2016年7月26日
 *      Author: Jugo
 */

#pragma once

#include <string>

struct cJSON;
class JSONObject;

using namespace std;

class JSONArray
{
public:
	JSONArray();
	JSONArray(cJSON *pcJSON);
	virtual ~JSONArray();
	void add(JSONObject &jsonObject);
	void add(JSONArray &jsonArray);
	void add(string name);
	void add(const char * name);
	void add(bool name);
	void add(int name);
	cJSON * getcJSON();
	cJSON * getJsonObject(int index);
	int size();
	bool getBoolean(int index);
	bool getBoolean(int index, bool defaultValue);
	int getInt(int index);
	int getInt(int index, int defaultValue);
	cJSON * getJsonArray(int index);
	string getString(int index);
	string getString(int index, string defaultValue);
	bool isNull(int index);
	string toString();
	void release();

private:
	cJSON * cjsonArray;
	int mnExtPointObj;
};
