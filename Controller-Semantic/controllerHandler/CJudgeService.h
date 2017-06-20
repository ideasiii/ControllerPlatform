/*
 * CJudgeService.h
 *
 *  Created on: 2017年6月9日
 *      Author: Jugo
 */

#pragma once

#include "CSemantic.h"
#include "CWeather.h"

class JSONObject;

class CJudgeService: public CSemantic
{
public:
	CJudgeService();
	virtual ~CJudgeService();

protected:
	std::string toString();
	int word(const char *szInput, JSONObject* jsonResp, std::map<std::string, std::string> &mapMatch);
	int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);

private:
	WEATHER weather;

private:
	void loadServiceDictionary();
	void getClock(std::string &strClock);
	void getWeather(const char *szLocation, WEATHER &weather);
	void getLocation(const char *szWord, WEATHER &weather);
	void getTranslate(const char *szWord, std::string &strResult);
};
