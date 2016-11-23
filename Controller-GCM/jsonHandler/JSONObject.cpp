/*
 * JSONObject.cpp
 *
 *  Created on: 2016年7月20日
 *      Author: cjsonObj
 */
#include <stdio.h>
#include <stdlib.h>
#include "JSONObject.h"
#include "cJSON.h"
#include "JSONArray.h"

using namespace std;

JSONObject::JSONObject() :
		cjsonObj(0)
{
	cjsonObj = cJSON_CreateObject();
}

JSONObject::JSONObject(string strSource) :
		cjsonObj(0)
{
	cjsonObj = cJSON_Parse(strSource.c_str());
	if (!cjsonObj)
	{
		printf("[JSONObject] Invalid JSON Source: %s", strSource.c_str());
	}
}

JSONObject::JSONObject(cJSON *pcJSON)
{
	cjsonObj = pcJSON;
}

JSONObject::~JSONObject()
{

}

void JSONObject::release()
{
	cJSON_Delete(cjsonObj);
}

bool JSONObject::isValid()
{
	if (cjsonObj)
		return true;
	return false;
}

string JSONObject::getString(string key)
{
	string strValue;

	if (cjsonObj)
	{
		if (cJSON_GetObjectItem(cjsonObj, key.c_str())
				&& (cJSON_String == cJSON_GetObjectItem(cjsonObj, key.c_str())->type))
		{
			strValue = cJSON_GetObjectItem(cjsonObj, key.c_str())->valuestring;
		}
	}

	return strValue;
}

int JSONObject::getInt(std::string key)
{
	int nValue = -1;
	if (cjsonObj)
	{
		if (cJSON_GetObjectItem(cjsonObj, key.c_str())
				&& (cJSON_Number == cJSON_GetObjectItem(cjsonObj, key.c_str())->type))
		{
			nValue = cJSON_GetObjectItem(cjsonObj, key.c_str())->valueint;
		}
	}

	return nValue;
}

string JSONObject::toString()
{
	return cJSON_Print(cjsonObj);
}

cJSON * JSONObject::getcJSON()
{
	return cjsonObj;
}

void JSONObject::put(std::string strKey, std::string strValue)
{
	cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateString(strValue.c_str()));
}

void JSONObject::put(std::string strKey, const char* szrValue)
{
	cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateString(szrValue));
}
void JSONObject::put(std::string strKey, int nValue)
{
	cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateNumber(nValue));
}

void JSONObject::put(std::string strKey, bool bValue)
{
	cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateBool(bValue));
}

void JSONObject::put(std::string strKey, double dValue)
{
	cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateNumber(dValue));
}

void JSONObject::put(std::string strKey, JSONObject &jsonObject)
{
	cJSON_AddItemToObject(cjsonObj, strKey.c_str(), jsonObject.getcJSON());
}

void JSONObject::put(std::string strKey, JSONArray &jsonArray)
{
	cJSON_AddItemToObject(cjsonObj, strKey.c_str(), jsonArray.getcJSON());
}

cJSON *JSONObject::getJsonArray(std::string strName)
{
	return cJSON_GetObjectItem(cjsonObj, strName.c_str());
}

cJSON *JSONObject::getJsonObject(std::string strName)
{
	return cJSON_GetObjectItem(cjsonObj, strName.c_str());
}

