/*
 * CFileHandler.h
 *
 *  Created on: 2017年5月26日
 *      Author: Jugo
 */

#pragma once

#include <vector>
#include <set>

class CObject;

class CFileHandler
{
public:
	CFileHandler();
	CFileHandler(CObject *object);
	virtual ~CFileHandler();
	unsigned int readAllLine(const char *szFile, std::vector<std::string> &setData);
	unsigned int readPath(const char *szPath, std::vector<std::string> &setData);
	unsigned int readAllLine(const char *szFile, std::set<std::string> &setData);
	unsigned int readPath(const char *szPath, std::set<std::string> &setData);
	unsigned int readContent(const char *szFile, std::string &strContent, bool bTrim = false);
	int copyFile(const char* src, const char* des);

private:
	CObject *mpController;
};
