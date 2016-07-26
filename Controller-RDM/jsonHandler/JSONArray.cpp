/*
 * JSONArray.cpp
 *
 *  Created on: 2016年7月26日
 *      Author: root
 */

#include "JSONArray.h"
#include "JSONObject.h"
#include "cJSON.h"

JSONArray::JSONArray() :
		cjsonArray(0)
{
	cjsonArray = cJSON_CreateArray();
}

JSONArray::JSONArray(cJSON *pcJSON)
{
	cjsonArray = pcJSON;
}

JSONArray::~JSONArray()
{

}

void JSONArray::add(JSONObject &jsonObject)
{
	cJSON_AddItemToArray(cjsonArray, jsonObject.getcJSON());
}

cJSON *JSONArray::getcJSON()
{
	return cjsonArray;
}

int JSONArray::size()
{
	return cJSON_GetArraySize(cjsonArray);
}

cJSON *JSONArray::getJsonObject(int index)
{
	return cJSON_GetArrayItem(cjsonArray, index);
}

