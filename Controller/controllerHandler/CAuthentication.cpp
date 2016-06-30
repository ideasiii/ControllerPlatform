/*
 * CAuthentication.cpp
 *
 *  Created on: 2016年3月30日
 *      Author: Jugo
 */

#include "../controllerHandler/CAuthentication.h"

#include "CSqliteHandler.h"
#include "packet.h"

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

	switch (nServiceType)
	{
		case TYPE_MOBILE_SERVICE:
			break;
		case TYPE_POWER_CHARGE_SERVICE:
			break;
		case TYPE_SDK_SERVICE:
			bAuth = sqlite->isAppIdExist(strData);
			break;
		case TYPE_TRACKER_SERVICE:
			break;
	}
	return bAuth;
}

