/*
 * CHttpsClient.h
 *
 *  Created on: 2017年4月25日
 *      Author: root
 */

#pragma once

#include <set>

class CHttpsClient
{
public:
	explicit CHttpsClient();
	virtual ~CHttpsClient();
	int GET(const char *szURL, std::string &strData, const std::set<std::string> &setHead);
	int POST(const char *szURL, std::string &strData, const std::set<std::string> &setHead,
			const std::set<std::string> &setParameter);

};

extern std::string urlEncode(const char *szStr);
//extern std::string url_encode(const std::string &value);
