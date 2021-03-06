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
		cjsonObj(0), mnExtPointObj(0)
{
	cjsonObj = cJSON_CreateObject();
}

JSONObject::JSONObject(string strSource) :
		cjsonObj(0), mnExtPointObj(0)
{
	cjsonObj = cJSON_Parse(strSource.c_str());
	if(!cjsonObj)
	{
		printf("[JSONObject] Invalid JSON Source: %s", strSource.c_str());
	}
}

JSONObject::JSONObject(cJSON *pcJSON) :
		mnExtPointObj(1)
{
	cjsonObj = pcJSON;
}

JSONObject::~JSONObject()
{

}

void JSONObject::release()
{
	if(mnExtPointObj)
		return;

	if(0 != cjsonObj)
	{
		cJSON_Delete(cjsonObj);
		cjsonObj = 0;
	}
}

bool JSONObject::isValid()
{
	if(cjsonObj)
		return true;
	return false;
}

bool JSONObject::getBoolean(string name)
{
	return getBoolean(name, false);
}

bool JSONObject::getBoolean(string name, bool defaultValue)
{
	int nValue = -1;
	if(cjsonObj)
	{
		if(!isNull(name) && (cJSON_Number == cJSON_GetObjectItem(cjsonObj, name.c_str())->type))
		{
			nValue = cJSON_GetObjectItem(cjsonObj, name.c_str())->valueint;
		}
	}
	if(-1 == nValue)
		return defaultValue;

	if(0 == nValue)
		return false;
	return true;
}

string JSONObject::getString(string name)
{
	string strValue;
	return getString(name, strValue);
}

string JSONObject::getString(string name, string defaultValue)
{
	string strValue = defaultValue;

	if(cjsonObj)
	{
		if(!isNull(name) && (cJSON_String == cJSON_GetObjectItem(cjsonObj, name.c_str())->type))
		{
			strValue = cJSON_GetObjectItem(cjsonObj, name.c_str())->valuestring;
		}
	}

	return strValue;
}

int JSONObject::getInt(string name)
{
	return getInt(name, -1);
}

int JSONObject::getInt(string name, int defaultValue)
{
	int nValue = defaultValue;
	if(cjsonObj)
	{
		if(!isNull(name) && (cJSON_Number == cJSON_GetObjectItem(cjsonObj, name.c_str())->type))
		{
			nValue = cJSON_GetObjectItem(cjsonObj, name.c_str())->valueint;
		}
	}
	return nValue;
}

bool JSONObject::isNull(string name)
{
	if(cJSON_GetObjectItem(cjsonObj, name.c_str()))
		return false;
	return true;
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

