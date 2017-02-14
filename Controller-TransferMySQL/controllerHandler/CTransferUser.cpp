/*
 * CTransferUser.cpp
 *
 *  Created on: 2017年1月16日
 *      Author: Jugo
 */

#include "CTransferUser.h"
#include "common.h"
#include "LogHandler.h"
#include "JSONArray.h"
#include "JSONObject.h"
#include "CPsqlHandler.h"
#include "CMysqlHandler.h"
#include "config.h"

using namespace std;

CTransferUser::CTransferUser() :
		pmysql(new CMysqlHandler)
{

}

CTransferUser::~CTransferUser()
{
	pmysql->close();
	delete pmysql;
}

int CTransferUser::start()
{

	_log("============== Transfer Uer Table Start ============");
	string strLastDate;
	CPsqlHandler psql;

	strLastDate = getMysqlLastDate();
	SETTING_DB psqlSeting;
	_DBG("psql host: %s", psqlSeting.strHost.c_str());
	_DBG("psql database: %s", psqlSeting.strDatabase.c_str());

	if (!psql.open(psqlSeting.strHost.c_str(), psqlSeting.strPort.c_str(), psqlSeting.strDatabase.c_str(),
			psqlSeting.strUser.c_str(), psqlSeting.strPassword.c_str()))
	{
		_log("[CTransferUser] Error: Postgresql Connect Fail");
		return FALSE;
	}

	string strSQL = "SELECT * FROM user WHERE create_date >= '" + strLastDate + "'";

	psql.close();

	return TRUE;
}

string CTransferUser::getMysqlLastDate()
{
	string strRet = DEFAULT_LAST_DATE;
	string strSQL;
	int nRet;

	list<map<string, string> > listRest;
	SETTING_DB mysqlSeting;
	nRet = pmysql->connect(mysqlSeting.strHost, mysqlSeting.strDatabase, mysqlSeting.strUser, mysqlSeting.strPassword);
	if (FALSE == nRet)
	{
		_log("[CTransferUser] %s", pmysql->getLastError());
	}
	else
	{
		strSQL = "select max(create_date) as maxdate from tracker_user";
		if (TRUE == pmysql->query(strSQL, listRest))
		{
			string strField;
			string strValue;
			map<string, string> mapItem;
			int nCount = 0;
			for (list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i)
			{
				mapItem = *i;
				for (map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
				{
					printf("%s : %s\n", (*j).first.c_str(), (*j).second.c_str());
					strRet = (*j).second.c_str();
				}
			}
		}
	}
	pmysql->close();
	return strRet;
}

