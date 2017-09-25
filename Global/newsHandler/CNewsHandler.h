/*
 * CNewsHandler.h
 *
 *  Created on: 2017年9月25日
 *      Author: jugo
 */

#pragma once

#include <string>

typedef struct _NEWS
{
	std::string strTitle;
	std::string strDescription;
	std::string strLink;
} NEWS;

class CNewsHandler
{
public:
	explicit CNewsHandler();
	virtual ~CNewsHandler();
	int getNews()
};
