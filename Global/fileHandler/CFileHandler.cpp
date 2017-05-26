/*
 * CFileHandler.cpp
 *
 *  Created on: 2017年5月26日
 *      Author: Jugo
 */

#include <fstream>
#include <string>
#include "CFileHandler.h"

using namespace std;

CFileHandler::CFileHandler() :
		mpController(0)
{

}

CFileHandler::CFileHandler(CObject *object)
{
	mpController = object;
}

CFileHandler::~CFileHandler()
{

}

unsigned int CFileHandler::readAllLine(const char *szFile, std::set<std::string> &setData)
{
	if(szFile)
	{
		ifstream file(szFile);
		string str;
		if(file.is_open())
		{
			while(getline(file, str))
				setData.insert(str);
			file.close();
		}
	}
	return setData.size();
}

