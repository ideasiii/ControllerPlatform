/*
 * CHttpsClient.h
 *
 *  Created on: 2017年4月25日
 *      Author: root
 */

#pragma once

class CHttpsClient
{
public:
	explicit CHttpsClient();
	virtual ~CHttpsClient();
	int GET(const char *szURL, std::string &strData);

};

extern std::string urlEncode(const char *szStr);
//extern std::string url_encode(const std::string &value);
