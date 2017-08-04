/*
 * JSONObject.h
 *
 *  Created on: 2016年7月20日
 *      Author: Jugo
 */

#pragma once

#include <string>

struct cJSON;
class JSONArray;

class JSONObject
{
public:
	explicit JSONObject();
	explicit JSONObject(std::string strSource);
	explicit JSONObject(const char *pSource);
	explicit JSONObject(cJSON *pcJSON);
	bool getBoolean(std::string name);
	bool getBoolean(std::string name, bool defaultValue);
	int getInt(std::string name);
	int getInt(std::string name, int defaultValue);
	float getFloat(std::string name);
	float getFloat(std::string name, float defaultValue);
	cJSON *getJsonArray(std::string strName);
	cJSON *getJsonObject(std::string strName);
	std::string getString(std::string name);
	std::string getString(std::string name, std::string defaultValue);
	bool isNull(std::string name);
	JSONObject &load(std::string strJSON);
	void create();

	virtual ~JSONObject();
	bool isValid();
	std::string toString();
	std::string toJSON();
	std::string toUnformattedString();
	JSONObject& put(std::string strKey, std::string strValue);
	JSONObject& put(std::string strKey, const char* szrValue);
	JSONObject& put(std::string strKey, int nValue);
	JSONObject& put(std::string strKey, bool bValue);
	JSONObject& put(std::string strKey, double dValue);
	JSONObject& put(std::string strKey, JSONObject &jsonObject);
	JSONObject& put(std::string strKey, JSONArray &jsonArray);

	// This function does not store the duplicate of, or the reference to jsonObject.
	// Instead it converts jsonObject to string and puts that string into collection.
	JSONObject& putSerialized(std::string strKey, JSONObject &jsonObject);

	cJSON * getcJSON();

	void release();

	void operator=(cJSON *c);

private:
	cJSON *cjsonObj;
	int mnExtPointObj;
};
