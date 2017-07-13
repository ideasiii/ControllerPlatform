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

class JSONArray
{
public:
	explicit JSONArray();
	explicit JSONArray(cJSON *pcJSON);
	virtual ~JSONArray();
	void add(JSONObject &jsonObject);
	void add(JSONArray &jsonArray);
	void add(std::string name);
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
	std::string getString(int index);
	std::string getString(int index, std::string defaultValue);
	bool isNull(int index);
	std::string toString();
	void release();
	bool isValid();
	JSONArray &load(cJSON *pcJSON);
	void create();
	void operator=(cJSON *c);
	std::string toJSON();

private:
	cJSON * cjsonArray;
	int mnExtPointObj;
};
