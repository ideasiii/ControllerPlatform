/*
 * JSONObject.cpp
 *
 *  Created on: 2016年7月20日
 *      Author: cjsonObj
 */
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include "JSONObject.h"
#include "cJSON.h"
#include "JSONArray.h"
#include "common.h"
#include "utility.h"
using namespace std;

JSONObject::JSONObject() :
		cjsonObj(0), mnExtPointObj(0)
{

}

JSONObject::JSONObject(string strSource) :
		cjsonObj(0), mnExtPointObj(0)
{
	if(!strSource.empty())
	{
		cjsonObj = cJSON_Parse(strSource.c_str());
		if(!cjsonObj)
		{
			printf("[JSONObject] Invalid JSON Source: %s", strSource.c_str());
		}
	}
}

JSONObject::JSONObject(const char *pSource) :
		cjsonObj(0), mnExtPointObj(0)
{
	if(pSource)
	{
		cjsonObj = cJSON_Parse(pSource);
		if(!cjsonObj)
		{
			printf("[JSONObject] Invalid JSON Source: %s", pSource);
		}
	}
}

JSONObject::JSONObject(cJSON *pcJSON) :
		mnExtPointObj(1)
{
	cjsonObj = pcJSON;
}

JSONObject::~JSONObject()
{
	release();
}

void JSONObject::create()
{
	release();
	mnExtPointObj = 0;
	cjsonObj = cJSON_CreateObject();
}

JSONObject &JSONObject::load(std::string strJSON)
{
	release();
	if(!strJSON.empty())
	{
		mnExtPointObj = 0;
		cjsonObj = cJSON_Parse(strJSON.c_str());
		if(!cjsonObj)
			printf("[JSONObject] load Invalid JSON Source: %s", strJSON.c_str());
	}
	return (*this);
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
	if(cjsonObj && !isNull(name))
	{
		int type = cJSON_GetObjectItem(cjsonObj, name.c_str())->type;
		return (cJSON_False == type) ? false : (cJSON_True == type) ? true : defaultValue;
	}

	return defaultValue;
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

float JSONObject::getFloat(string name)
{
	return getFloat(name, 0.0);
}

float JSONObject::getFloat(string name, float defaultValue)
{
	float fValue = defaultValue;
	if(cjsonObj)
	{
		if(!isNull(name) && (cJSON_Number == cJSON_GetObjectItem(cjsonObj, name.c_str())->type))
		{
			fValue = cJSON_GetObjectItem(cjsonObj, name.c_str())->valueint;
		}
	}
	return fValue;
}

bool JSONObject::isNull(string name)
{
	if(cJSON_GetObjectItem(cjsonObj, name.c_str()))
		return false;
	return true;
}

string JSONObject::toString()
{
	return ObjectToString(cjsonObj);
}

string JSONObject::toJSON()
{
	string strJSON = trim(ObjectToString(cjsonObj));
	strJSON = ReplaceAll(strJSON, "\"{", "{");
	strJSON = ReplaceAll(strJSON, "}\"", "}");
	strJSON = ReplaceAll(strJSON, "\"[", "[");
	strJSON = ReplaceAll(strJSON, "]\"", "]");
	strJSON = ReplaceAll(strJSON, "\\\"", "\"");
	return strJSON;
}

string JSONObject::toUnformattedString()
{
	if(cjsonObj == NULL)
	{
		return "{\"nullSource\":null}";
	}

	char *out = cJSON_PrintUnformatted(cjsonObj);
	std::string strOut(out);
	free(out);
	return strOut;
}

cJSON * JSONObject::getcJSON()
{
	return cjsonObj;
}

JSONObject& JSONObject::put(std::string strKey, std::string strValue)
{
	if(isNull(strKey))
		cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateString(strValue.c_str()));
	else
		cJSON_ReplaceItemInObject(cjsonObj, strKey.c_str(), cJSON_CreateString(strValue.c_str()));
	return (*this);
}

JSONObject& JSONObject::put(std::string strKey, const char* szrValue)
{
	if(isNull(strKey))
		cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateString(szrValue));
	else
		cJSON_ReplaceItemInObject(cjsonObj, strKey.c_str(), cJSON_CreateString(szrValue));
	return (*this);
}
JSONObject& JSONObject::put(std::string strKey, int nValue)
{
	if(isNull(strKey))
		cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateNumber(nValue));
	else
		cJSON_ReplaceItemInObject(cjsonObj, strKey.c_str(), cJSON_CreateNumber(nValue));
	return (*this);
}

JSONObject& JSONObject::put(std::string strKey, bool bValue)
{
	if(isNull(strKey))
		cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateBool(bValue));
	else
		cJSON_ReplaceItemInObject(cjsonObj, strKey.c_str(), cJSON_CreateBool(bValue));
	return (*this);
}

JSONObject& JSONObject::put(std::string strKey, double dValue)
{
	if(isNull(strKey))
		cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateNumber(dValue));
	else
		cJSON_ReplaceItemInObject(cjsonObj, strKey.c_str(), cJSON_CreateNumber(dValue));
	return (*this);
}

JSONObject& JSONObject::put(std::string strKey, JSONObject &jsonObject)
{
	if(isNull(strKey))
		cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_Duplicate(jsonObject.getcJSON(), 1));
	else
		cJSON_ReplaceItemInObject(cjsonObj, strKey.c_str(), cJSON_Duplicate(jsonObject.getcJSON(), 1));
	return (*this);
}

JSONObject& JSONObject::put(std::string strKey, JSONArray &jsonArray)
{
	if(isNull(strKey))
		cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_Duplicate(jsonArray.getcJSON(), 1));
	else
		cJSON_ReplaceItemInObject(cjsonObj, strKey.c_str(), cJSON_Duplicate(jsonArray.getcJSON(), 1));
	return (*this);
}

JSONObject& JSONObject::putSerialized(std::string strKey, JSONObject &jsonObject)
{
	if(isNull(strKey))
		cJSON_AddItemToObject(cjsonObj, strKey.c_str(), cJSON_CreateSerializedObject(jsonObject.getcJSON()));
	else
		cJSON_ReplaceItemInObject(cjsonObj, strKey.c_str(), cJSON_CreateSerializedObject(jsonObject.getcJSON()));
	return (*this);
}

cJSON *JSONObject::getJsonArray(std::string strName)
{
	return cJSON_GetObjectItem(cjsonObj, strName.c_str());
}

cJSON *JSONObject::getJsonObject(std::string strName)
{
	return cJSON_GetObjectItem(cjsonObj, strName.c_str());
}

void JSONObject::operator=(cJSON *c)
{
	release();
	mnExtPointObj = 1;
	cjsonObj = c;
}
