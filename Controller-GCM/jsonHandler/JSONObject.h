/*
 * JSONObject.h
 *
 *  Created on: 2016年7月20日
 *      Author: Jugo
 */

#pragma once
#include <string>

class cJSON;
class JSONArray;

class JSONObject
{
public:
	explicit JSONObject();
	explicit JSONObject(std::string strSource);
	explicit JSONObject(cJSON *pcJSON);
	std::string getString(std::string key);
	int getInt(std::string key);
	virtual ~JSONObject();
	bool isValid();
	std::string toString();
	void put(std::string strKey, std::string strValue);
	void put(std::string strKey, const char* szrValue);
	void put(std::string strKey, int nValue);
	void put(std::string strKey, bool bValue);
	void put(std::string strKey, double dValue);
	void put(std::string strKey, JSONObject &jsonObject);
	void put(std::string strKey, JSONArray &jsonArray);
	cJSON * getcJSON();
	cJSON *getJsonObject(std::string strName);
	cJSON *getJsonArray(std::string strName);
	void release();

private:
	cJSON *cjsonObj;
};
