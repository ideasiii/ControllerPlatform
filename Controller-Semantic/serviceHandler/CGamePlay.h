/*
 * CGamePlay.h
 *
 *  Created on: 2017年8月24日
 *      Author: Jugo
 */

#pragma once

#include "JSONObject.h"

class CGamePlay
{
public:
	CGamePlay();
	virtual ~CGamePlay();
	int activity(const char *szInput, JSONObject& jsonResp);
};
