/*
 * CRdmLogin.cpp
 *
 *  Created on: 2016年7月21日
 *      Author: root
 */

#include "CRdmLogin.h"
#include "CSqliteHandler.h"
#include "LogHandler.h"
#include <list>

using namespace std;

CRdmLogin::CRdmLogin()
{

}

CRdmLogin::~CRdmLogin()
{

}

bool CRdmLogin::login(const std::string strAccount, const std::string strPassword, const std::string strId,
		const int nDevice, const string strModel)
{
	bool bResult = false;
	int nResult = R_SQLITE_ERROR;
	string strSql;

	strSql = "select group_id from group_info where account = '" + strAccount + "' and password = '" + strPassword
			+ "'";
	list<string> listValue;
	CSqliteHandler::getInstance()->mdmAndroidSqlExec(strSql.c_str(), listValue, 0);

	string strGroupId;
	list<string>::iterator it = listValue.begin();
	for (list<string>::iterator it = listValue.begin(); it != listValue.end(); ++it)
	{
		strGroupId = *it;
		_log("[CRdmLogin] Get Group Id: %s", strGroupId.c_str());
		break;
	}

	if (!strGroupId.empty())
	{
		if (R_SQLITE_OK == logout(strId))
		{
			strSql = "insert into device_info(mac_address,device_model,group_id) values('" + strId + "','" + strModel
					+ "','" + strGroupId + "')";
			if (R_SQLITE_OK == CSqliteHandler::getInstance()->mdmAndroidSqlExec(strSql.c_str()))
			{
				bResult = true;
			}
		}
	}

	return bResult;
}

int CRdmLogin::logout(const string strId)
{
	string strSql = "delete from device_info where mac_address = '" + strId + "'";
	return CSqliteHandler::getInstance()->mdmAndroidSqlExec(strSql.c_str());
}
