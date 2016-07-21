/*
 * JSONObject.h
 *
 *  Created on: 2016年7月20日
 *      Author: Jugo
 */

#pragma once
#include <string>

class cJSON;

class JSONObject
{
public:
	explicit JSONObject();
	explicit JSONObject(std::string strSource);
	std::string getString(std::string key);
	int getInt(std::string key);
	virtual ~JSONObject();
	bool isValid();

private:
	cJSON *root;
};
