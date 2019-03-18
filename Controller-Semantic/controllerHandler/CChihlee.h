/*
 * CChihlee.h
 *
 *  Created on: 2019年3月15日
 *      Author: jugo
 */

#pragma once

class JSONObject;

class CChihlee
{
public:
	explicit CChihlee();
	virtual ~CChihlee();
	void runAnalysis(const char *szInput, JSONObject &jsonResp);

private:
	void playSound(const char *szWav);
};
