/*
 * CFileHandler.h
 *
 *  Created on: 2017年5月26日
 *      Author: Jugo
 */

#pragma once

#include <set>

class CObject;

class CFileHandler
{
public:
	CFileHandler();
	CFileHandler(CObject *object);
	virtual ~CFileHandler();
	unsigned int readAllLine(const char *szFile, std::set<std::string> &setData);

private:
	CObject *mpController;
};
