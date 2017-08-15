/*
 * CPenReader.h
 *
 *  Created on: 2017年8月15日
 *      Author: Jugo
 */

#pragma once

#include "JSONObject.h"

class CPenReader
{
public:
	CPenReader();
	virtual ~CPenReader();
	int activity(const char *szInput, JSONObject& jsonResp);
};
