/*
 * JSONObject.cpp
 *
 *  Created on: 2016年7月20日
 *      Author: root
 */

#include "JSONObject.h"
#include "cJSON.h"
#include "LogHandler.h"

using namespace std;

JSONObject::JSONObject() :
		root(0)
{

}

JSONObject::JSONObject(string strSource) :
		root(0)
{
	root = cJSON_Parse(strSource.c_str());
	if (!root)
	{
		_log("[JSONObject] Invalid JSON Source: %s", strSource.c_str());
	}
}

JSONObject::~JSONObject()
{
	cJSON_Delete(root);
}

bool JSONObject::isValid()
{
	if (root)
		return true;
	return false;
}

string JSONObject::getString(string key)
{
	string strValue;

	if (root)
	{
		if (cJSON_GetObjectItem(root, key.c_str()) && (cJSON_String == cJSON_GetObjectItem(root, key.c_str())->type))
		{
			strValue = cJSON_GetObjectItem(root, key.c_str())->valuestring;
		}
	}

	return strValue;
}

int JSONObject::getInt(std::string key)
{
	int nValue = -1;
	if (root)
	{
		if (cJSON_GetObjectItem(root, key.c_str()) && (cJSON_Number == cJSON_GetObjectItem(root, key.c_str())->type))
		{
			nValue = cJSON_GetObjectItem(root, key.c_str())->valueint;
		}
	}

	return nValue;
}

