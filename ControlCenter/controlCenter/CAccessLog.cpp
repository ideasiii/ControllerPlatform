/*
 * CAccessLog.cpp
 *
 *  Created on: 2015年12月7日
 *      Author: Louis Ju
 */

#include "common.h"
#include "CAccessLog.h"
#include "packet.h"
#include <map>
#include "utility.h"
#include "CMongoDBHandler.h"

using namespace std;

static CAccessLog* mInstance = 0;

CAccessLog::CAccessLog() :
		mongodb(CMongoDBHandler::getInstance())
{

}

CAccessLog::~CAccessLog()
{

}

CAccessLog* CAccessLog::getInstance()
{
	if (0 == mInstance)
	{
		mInstance = new CAccessLog();
	}
	return mInstance;
}

string CAccessLog::insertLog(const int nType, string strData)
{
	string strOID;
	switch (nType)
	{
		case TYPE_MOBILE_SERVICE:
			strOID = mongodb->insert("access", "mobile", strData);
			break;
		case TYPE_POWER_CHARGE_SERVICE:
			strOID = mongodb->insert("access", "power", strData);
			break;
		case TYPE_SDK_SERVICE:
			strOID = mongodb->insert("access", "sdk", strData);
			break;
		case TYPE_TRACKER_SERVICE:
			strOID = mongodb->insert("access", "tracker", strData);
			break;
		case TYPE_TRACKER_APPLIENCE:
			strOID = mongodb->insert("access", "applience", strData);
			break;
		case TYPE_TRACKER_TOY:
			strOID = mongodb->insert("access", "toy", strData);
			break;
		case TYPE_TRACKER_IOT:
			strOID = mongodb->insert("access", "iot", strData);
			break;
		default:
			//strOID = mongodb->insert("access", "unknow", strData);
			log("Insert Access log fail, unknow service type", "[AccessLog]");
			break;
	}
	return strOID;
}
