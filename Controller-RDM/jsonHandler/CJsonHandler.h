/*
 * CJsonHandler.h
 *
 *  Created on: 2016年7月19日
 *      Author: Jugo
 */

#pragma once

#include <string>

class CJsonHandler
{
public:
	CJsonHandler();
	virtual ~CJsonHandler();

	static std::string getJsonString(std::string strJSON, std::string strName);
	static int getJsonInt(std::string strJSON, std::string strName);
};
