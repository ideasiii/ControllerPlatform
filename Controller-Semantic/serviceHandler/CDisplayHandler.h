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
	enum DISPLAY
	{
		DEFAULT = 0, SAD, HAPPY
	};
public:
	CDisplayHandler();
	virtual ~CDisplayHandler();
	std::string getDefaultDisplay();
	std::string getSadDisplay();
	std::string getHappyDisplay();

private:
	std::map<int, std::string> mapDisplayFile;
	std::string getDisplay(int nDisplay);
};
