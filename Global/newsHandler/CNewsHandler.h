/*
 * CNewsHandler.h
 *
 *  Created on: 2017年9月25日
 *      Author: jugo
 */

#pragma once

#include <string>
#include <list>

typedef struct _NEWS
{
	std::string strTitle;
	std::string strDescription;
	std::string strLink;
} NEWS;

typedef struct _NEWS_DATE
{
	std::string strDate;
	std::list<NEWS> listNews;
} NEWS_DATE;

class CNewsHandler
{
public:
	explicit CNewsHandler();
	virtual ~CNewsHandler();
	int getNewsToday(NEWS_DATE &newDate);
};
