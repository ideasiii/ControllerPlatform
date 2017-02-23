/*
 * CTransferUser.cpp
 *
 *  Created on: 2017年1月16日
 *      Author: Jugo
 */

#include <list>
#include <map>
#include "CTransferUser.h"
#include "common.h"
#include "LogHandler.h"
#include "CPsqlHandler.h"
#include "CMysqlHandler.h"
#include "config.h"

using namespace std;

CTransferUser::CTransferUser() :
		pmysql(new CMysqlHandler), ppsql(new CPsqlHandler)
{

}

CTransferUser::~CTransferUser()
{
	pmysql->close();
	delete pmysql;

	ppsql->close();
	delete ppsql;
}

int CTransferUser::start()
{

	_log("============== Transfer Uer Table Start ============");

	extern map<string, string> mapPsqlSetting;
	extern map<string, string> mapMysqlSetting;
	string strLastDate;
	string strSQL;
	string strValues;
	string strId;
	int nRet;
	map<string, string> mapItem;
	//list<map<string, string> > listRestMysqlId;

	strLastDate = getMysqlLastDate();
	if (strLastDate.empty())
	{
		_log("[CTransferUser] get mysql last create_date fail");
		return FALSE;
	}

	if (!ppsql->open(mapPsqlSetting["host"].c_str(), mapPsqlSetting["port"].c_str(), mapPsqlSetting["database"].c_str(),
			mapPsqlSetting["user"].c_str(), mapPsqlSetting["password"].c_str()))
	{
		_log("[CTransferUser] Error: Postgresql Connect Fail");
		return FALSE;
	}

	nRet = pmysql->connect(mapMysqlSetting["host"], mapMysqlSetting["database"], mapMysqlSetting["user"],
			mapMysqlSetting["password"]);
	if (FALSE == nRet)
	{
		_log("[CTransferUser] Mysql Error: %s", pmysql->getLastError().c_str());
		ppsql->close();
		return FALSE;
	}
#ifdef SYNCALL_USER
	strSQL = "SELECT * FROM tracker_user";
#else
	strSQL = "SELECT * FROM tracker_user WHERE create_date >= '" + strLastDate + "'";
#endif
	_log("[CTransferUser] run PSQL: %s", strSQL.c_str());

	list<map<string, string> > listRest;
	ppsql->query(strSQL.c_str(), listRest);
	ppsql->close();

	_log("[CTransferUser] Query PSQL Table tracker_user Count: %d to Insert.", listRest.size());
	int nCount = 0;
	for (list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i, ++nCount)
	{
		strSQL = "INSERT INTO tracker_user (";
		strValues = "VALUES(";
		mapItem = *i;
		for (map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
		{
			strSQL += (*j).first;
			strValues = strValues + "'" + (*j).second + "'";
			if (mapItem.end() != ++j)
			{
				strSQL += ",";
				strValues += ",";
			}
			else
			{
				strSQL += ") ";
				strValues += ")";
			}
			--j;
			if (0 == (*j).first.compare("id"))
			{
				strId = (*j).second;
			}
		}

		strSQL += strValues;

		if (FALSE == pmysql->sqlExec(strSQL))
		{
			if (1062 != pmysql->getLastErrorNo())
				_log("[CTransferUser] Mysql sqlExec Error: %s", pmysql->getLastError().c_str());
		}
		else
		{
			_log("[CTransferUser] run MYSQL: %s", strSQL.c_str());
		}
	}
	pmysql->close();

	_log("[CTransferUser] Mysql Table tracker_user Total run insert Count: %d", nCount);

	return TRUE;
}

string CTransferUser::getMysqlLastDate()
{
	string strRet;
	string strSQL;
	int nRet;

	list<map<string, string> > listRest;
	extern map<string, string> mapMysqlSetting;
	nRet = pmysql->connect(mapMysqlSetting["host"], mapMysqlSetting["database"], mapMysqlSetting["user"],
			mapMysqlSetting["password"]);
	if (FALSE == nRet)
	{
		_log("[CTransferUser] getMysqlLastDate Mysql Error: %s", pmysql->getLastError().c_str());
	}
	else
	{
		strSQL = "select max(create_date) as maxdate from tracker_user";

		if (TRUE == pmysql->query(strSQL, listRest))
		{
			strRet = DEFAULT_LAST_DATE;
			string strField;
			string strValue;
			map<string, string> mapItem;
			int nCount = 0;
			for (list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i)
			{
				mapItem = *i;
				for (map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
				{
					//printf("%s : %s\n", (*j).first.c_str(), (*j).second.c_str());
					strRet = (*j).second.c_str();
				}
			}
		}
	}
	pmysql->close();

	return strRet;
}

