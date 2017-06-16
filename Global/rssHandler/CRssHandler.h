/*
 * CRssHandler.h
 *
 *  Created on: 2017年6月16日
 *      Author: Jugo
 */

#pragma once

typedef struct _RSS
{
	std::string strTitle;
	std::string strDescription;
	std::string strLink;
	std::string strPubDate;
} RSS;
class CRssHandler
{
public:
	CRssHandler();
	virtual ~CRssHandler();
	void getItems(const char *szURL, std::map<int, RSS> &mapRSS);
};
