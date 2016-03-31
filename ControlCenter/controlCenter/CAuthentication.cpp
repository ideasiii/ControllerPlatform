/*
 * CAuthentication.cpp
 *
 *  Created on: 2016年3月30日
 *      Author: Jugo
 */

#include "CAuthentication.h"
#include "CSqliteHandler.h"

using namespace std;

static CAuthentication *instance = 0;

CAuthentication::CAuthentication() :
		sqlite(CSqliteHandler::getInstance())
{

}

CAuthentication::~CAuthentication()
{

}

CAuthentication* CAuthentication::getInstance()
{
	if (0 == instance)
	{
		instance = new CAuthentication;
	}

	return instance;
}

bool CAuthentication::authorization(const int nServiceType, const std::string strData)
{
	bool bAuth = false;

	return bAuth;
}

