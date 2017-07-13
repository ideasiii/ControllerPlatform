/*
 * JSONArray.cpp
 *
 *  Created on: 2016年7月26日
 *      Author: root
 */

#include <stdio.h>
#include "JSONArray.h"
#include "JSONObject.h"
#include "cJSON.h"
#include "utility.h"

using namespace std;

JSONArray::JSONArray() :
		cjsonArray(0), mnExtPointObj(0)
{

}

JSONArray::JSONArray(cJSON *pcJSON) :
		mnExtPointObj(1)
{
	cjsonArray = pcJSON;
	if(cjsonArray)
		cjsonArray->type = cJSON_Array;
}

void JSONArray::create()
{
	mnExtPointObj = 0;
	cjsonArray = cJSON_CreateArray();
}

JSONArray::~JSONArray()
{
	release();
}

JSONArray &JSONArray::load(cJSON *pcJSON)
{
	release();
	cjsonArray = pcJSON;
	if(cjsonArray)
	{
		mnExtPointObj = 1;
		cjsonArray->type = cJSON_Array;
	}
	else
		printf("[JSONArray] load Invalid JSON Source");
	return (*this);
}

bool JSONArray::isValid()
{
	if(cjsonArray)
		return true;
	return false;
}

void JSONArray::add(JSONObject &jsonObject)
{
	cJSON_AddItemToArray(cjsonArray, cJSON_Duplicate(jsonObject.getcJSON(), 1));
}

void JSONArray::add(JSONArray &jsonArray)
{
	cJSON_AddItemToArray(cjsonArray, cJSON_Duplicate(jsonArray.getcJSON(), 1));
}

void JSONArray::add(string name)
{
	cJSON_AddItemToArray(cjsonArray, cJSON_CreateString(name.c_str()));
}

void JSONArray::add(const char * name)
{
	cJSON_AddItemToArray(cjsonArray, cJSON_CreateString(name));
}

void JSONArray::add(bool name)
{
	cJSON_AddItemToArray(cjsonArray, cJSON_CreateBool(name ? 1 : 0));
}
void JSONArray::add(int name)
{
	cJSON_AddItemToArray(cjsonArray, cJSON_CreateNumber(name));
}

cJSON *JSONArray::getcJSON()
{
	return cjsonArray;
}

int JSONArray::size()
{
	if(!cjsonArray)
		return 0;
	return cJSON_GetArraySize(cjsonArray);
}

cJSON *JSONArray::getJsonObject(int index)
{
	return cJSON_GetArrayItem(cjsonArray, index);
}

bool JSONArray::getBoolean(int index)
{
	return getBoolean(index, false);
}

bool JSONArray::getBoolean(int index, bool defaultValue)
{
	bool bValue = defaultValue;

	cJSON *value = cJSON_GetArrayItem(cjsonArray, index);
	if(value && cJSON_Number == value->type)
	{
		if(0 == value->valueint)
			bValue = false;
		else
			bValue = true;
	}
	return bValue;
}

int JSONArray::getInt(int index)
{
	return getInt(index, 0);
}
int JSONArray::getInt(int index, int defaultValue)
{
	int nValue = defaultValue;
	cJSON *value = cJSON_GetArrayItem(cjsonArray, index);
	if(value && cJSON_Number == value->type)
	{
		nValue = value->valueint;
	}
	return nValue;
}

cJSON * JSONArray::getJsonArray(int index)
{
	return cJSON_GetArrayItem(cjsonArray, index);
}

string JSONArray::getString(int index)
{
	return getString(index, string());
}

string JSONArray::getString(int index, string defaultValue)
{
	string strValue = defaultValue;
	cJSON *value = cJSON_GetArrayItem(cjsonArray, index);
	if(value && cJSON_String == value->type)
	{
		strValue = value->valuestring;
	}
	return strValue;
}

bool JSONArray::isNull(int index)
{
	cJSON *value = cJSON_GetArrayItem(cjsonArray, index);
	if(value)
		return true;
	return false;
}

string JSONArray::toString()
{
	JSONObject jsonItem;
	string strOut = "[";

	for(int i = 0; i < this->size(); ++i)
	{
		jsonItem = this->getJsonObject(i);
		strOut += jsonItem.toString();
		if(i != this->size() - 1)
		{
			strOut += ",";
		}
	}
	strOut += "]";
	return strOut;
}

string JSONArray::toJSON()
{

	JSONObject jsonItem;
	string strOut = "[";

	for(int i = 0; i < this->size(); ++i)
	{
		jsonItem = this->getJsonObject(i);
		strOut += jsonItem.toJSON();
		if(i != this->size() - 1)
		{
			strOut += ",";
		}
	}
	strOut += "]";

	return trim(strOut);
}

void JSONArray::release()
{
	if(mnExtPointObj)
		return;
	if(0 != cjsonArray)
	{
		cJSON_Delete(cjsonArray);
		cjsonArray = 0;
	}
}

void JSONArray::operator=(cJSON *c)
{
	mnExtPointObj = 1;
	cjsonArray = c;
}

