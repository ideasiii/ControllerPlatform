/*
 * main.cpp
 *
 *  Created on: 2017年01月05日
 *      Author: Jugo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <map>

#include "CMysqlHandler.h"

using namespace std;

int main()
{
	CMysqlHandler *pmysql = new CMysqlHandler();
	string strSQL;

	pmysql->connect("175.98.119.121", "tracker", "ideas", "tracker123!", "5");
	//strSQL =
	//		"insert into tracker_user(id,app_id,mac,os,phone,fb_id,fb_name,fb_email,fb_account,g_account,t_account,create_date) values('0123456789','414515787','AABBCCDD','Android','','','','','','','','2017-02-14 11:49:00')";
	//pmysql->sqlExec(strSQL);

	strSQL = "select * from app";
	list<map<string, string> > listRest;
	pmysql->query(strSQL, listRest);

	string strField;
	string strValue;
	map<string, string> mapItem;
	int nCount = 0;
	for(list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i, ++nCount)
	{
		mapItem = *i;
		for(map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
		{
			printf("%s : %s\n", (*j).first.c_str(), (*j).second.c_str());
		}
	}
	printf("=============================%d================================\n", nCount);

	pmysql->close();

	delete pmysql;
	return 0;
}
