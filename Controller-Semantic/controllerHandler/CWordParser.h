/*
 * CWordParser.h
 *
 *  Created on: 2016年10月6日
 *      Author: root
 */

#pragma once

#include <string>

class JSONObject;
class CDelivery;

class CWordParser
{
public:
	CWordParser();
	virtual ~CWordParser();
	static int parser(std::string strWord, JSONObject &jsonOutput);

private:

};
