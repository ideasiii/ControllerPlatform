/*
 * CDisplayHandler.h
 *
 *  Created on: 2017年8月24日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <map>

class CDisplayHandler
{
public:
	CDisplayHandler();
	virtual ~CDisplayHandler();
	std::string getDefaultDisplay();

private:
	std::map<int, std::string> mapDisplayFile;
};
