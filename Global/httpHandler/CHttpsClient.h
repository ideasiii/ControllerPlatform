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
	int GET(const char *szURL);
};
