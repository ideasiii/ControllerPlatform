/*
 * JSONArray.h
 *
 *  Created on: 2016年7月26日
 *      Author: Jugo
 */

#pragma once

#include <string>

class cJSON;
class JSONObject;

class JSONArray
{
public:
	JSONArray();
	JSONArray(cJSON *pcJSON);
	virtual ~JSONArray();
	void add(JSONObject &jsonObject);
	cJSON *getcJSON();
	cJSON *getJsonObject(int index);
	int size();

private:
	cJSON *cjsonArray;
};
