/*
 * CMysqlHandler.cpp
 *
 *  Created on: 2017年2月13日
 *      Author: jugo
 */

#include <iostream>

#include "CMysqlHandler.h"
#include "common.h"

using namespace std;

CMysqlHandler::CMysqlHandler()
{
	mpMySQL = mysql_init(NULL);
	if (NULL == mpMySQL)
	{
		setError("MySQL Init Fail");
	}
	else
		cout << "[CMysqlHandler] MySQL Init Success" << endl;
}

CMysqlHandler::~CMysqlHandler()
{
	close();
}

int CMysqlHandler::connect(string strHost, string strDB, string strUser, string strPassword)
{
	if (NULL == mpMySQL)
	{
		setError("MySQL Client not Init");
		return FALSE;
	}

	cout << "mysql stat: " << mysql_stat(mpMySQL) << endl;

	// 函數mysql_real_connect建立一個數據庫連接
	// 成功返回MYSQL*連接句柄，失敗返回NULL
	mpMySQL = mysql_real_connect(mpMySQL, strHost.c_str(), strUser.c_str(), strPassword.c_str(), strDB.c_str(), 0, NULL,
			0);
	if ( NULL == mpMySQL)
	{
		setError("MySQL Connect Fail");
		return FALSE;
	}

	cout << "[CMysqlHandler] MySQL Connect Success!!" << endl;
	return TRUE;
}

void CMysqlHandler::close()
{
	if (NULL != mpMySQL)
	{
		mysql_close(mpMySQL);
		mpMySQL = NULL;
		cout << "[CMysqlHandler] MySQL Close" << endl;
	}
}

void CMysqlHandler::setError(string strMsg)
{
	if (NULL != mpMySQL)
	{
		mstrLastError = mysql_error(mpMySQL);
		cout << "[CMysqlHandler] " << strMsg << ": " << mstrLastError << endl;
	}
}

string CMysqlHandler::getLastError()
{
	return mstrLastError;
}

int CMysqlHandler::sqlExec(string strSQL)
{
	if ( NULL == mpMySQL)
	{
		setError("MySQL Connector Invalid");
		return FALSE;
	}

	// mysql_query()執行成功返回0，失敗返回非0值。
	if (mysql_query(mpMySQL, strSQL.c_str()))
	{
		setError("Query Error");
		return FALSE;
	}

	return TRUE;
}

int CMysqlHandler::query(string strSQL, list<map<string, string> > &listRest)
{
	if ( NULL == mpMySQL)
	{
		setError("MySQL Connector Invalid");
		return FALSE;
	}

	// mysql_query()執行成功返回0，失敗返回非0值。
	if (mysql_query(mpMySQL, strSQL.c_str()))
	{
		setError("Query Error");
		return FALSE;
	}
	else
	{
		MYSQL_RES *result = mysql_use_result(mpMySQL); // 獲取結果集
		MYSQL_ROW row;
		// mysql_field_count()返回connection查詢的列數
		for (unsigned int i = 0; i < mysql_field_count(mpMySQL); ++i)
		{
			// 獲取下一行
			row = mysql_fetch_row(result);
			if (row <= 0)
			{
				break;
			}
			// mysql_num_fields()返回結果集中的字段數
			for (unsigned int j = 0; j < mysql_num_fields(result); ++j)
			{
				cout << row[j] << " ";
			}
			cout << endl;
		}
		// 釋放結果集的內存
		mysql_free_result(result);
	}
	return TRUE;
}

