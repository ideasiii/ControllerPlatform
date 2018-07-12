/*
 * CMysqlHandler.cpp
 *
 *  Created on: 2017年2月13日
 *      Author: jugo
 */

#include "CMysqlHandler.h"
#include "common.h"
#include "LogHandler.h"

using namespace std;

CMysqlHandler::CMysqlHandler() :
		mnLastErrorNo(0)
{
	mpMySQL = 0;
}

CMysqlHandler::~CMysqlHandler()
{
	close();
}

bool CMysqlHandler::isValid()
{
	if(mpMySQL)
		return true;
	return false;
}
int CMysqlHandler::connect(string strHost, string strDB, string strUser, string strPassword, const char *szConnTimeout)
{
	close();
	mpMySQL = mysql_init(NULL);
	if(NULL == mpMySQL)
	{
		setError("MySQL Init Fail");
		return FALSE;
	}

	if(szConnTimeout)
		mysql_options(mpMySQL, MYSQL_OPT_CONNECT_TIMEOUT, szConnTimeout);
	mysql_options(mpMySQL, MYSQL_SET_CHARSET_NAME, "utf8");
	mysql_options(mpMySQL, MYSQL_INIT_COMMAND, "SET NAMES utf8");

	// 函數mysql_real_connect建立一個數據庫連接
	// 成功返回MYSQL*連接句柄，失敗返回NULL
	//	_DBG("[CMysqlHandler] connect: host=%s user=%s password=%s database=%s", strHost.c_str(), strUser.c_str(),
	//			strPassword.c_str(), strDB.c_str());
	//mysql_real_connect(mpMySQL, strHost.c_str(), strUser.c_str(), strPassword.c_str(), strDB.c_str(), 0, NULL, 0);
	if(!mysql_real_connect(mpMySQL, strHost.c_str(), strUser.c_str(), strPassword.c_str(), strDB.c_str(), 0, NULL, 0))
	{
		setError("MySQL Connect Fail");
		close();
		return FALSE;
	}

	_log("[CMysqlHandler] MySQL Connect Success!!");
	return TRUE;

}

void CMysqlHandler::close()
{
	if(NULL != mpMySQL)
	{
		mysql_close(mpMySQL);
		mpMySQL = NULL;
		_log("[CMysqlHandler] MySQL Close");
	}
}

/**
 * Error: 1062 SQLSTATE: 23000 (ER_DUP_ENTRY)
 */
void CMysqlHandler::setError(string strMsg)
{
//	if(NULL != mpMySQL)
//	{
	mstrLastError = mysql_error(mpMySQL);
	mnLastErrorNo = mysql_errno(mpMySQL);
	_log("[CMysqlHandler] setError Error:%s mysql_error:%s", strMsg.c_str(), mstrLastError.c_str());
//	}
}

string CMysqlHandler::getLastError()
{
	return mstrLastError;
}

int CMysqlHandler::getLastErrorNo()
{
	return mnLastErrorNo;
}

int CMysqlHandler::sqlExec(string strSQL)
{

	if( NULL == mpMySQL)
	{
		setError("MySQL Connector Invalid");
		return FALSE;
	}

	//如果執行成功，返回0。如果出现错误，返回非0值。
	if(mysql_real_query(mpMySQL, strSQL.c_str(), strSQL.length()))
	{
		setError("sqlExec Error");
		return FALSE;
	}

	return TRUE;
}

int CMysqlHandler::query(string strSQL, list<map<string, string> > &listRest)
{
	if( NULL == mpMySQL)
	{
		setError("MySQL Connector Invalid");
		return FALSE;
	}

	// mysql_query()執行成功返回0，失敗返回非0值。
	_log("[CMysqlHandler] mysql_query: %s", strSQL.c_str());
	if(mysql_real_query(mpMySQL, strSQL.c_str(), strSQL.length()))
	{
		setError("Query Error");
		return FALSE;
	}
	else
	{
		MYSQL_RES *result = mysql_use_result(mpMySQL); // 獲取結果集
		if(NULL == result)
		{
			setError("MySQL Result Fail");
			return FALSE;
		}

		MYSQL_FIELD *fields = mysql_fetch_fields(result); // 獲取欄位
		MYSQL_ROW row;

		//fields = mysql_fetch_fields(result);
		map<string, string> dataItem;
		string strItem;
		string strField;

		while((row = mysql_fetch_row(result)))
		{
			// 獲取下一行
			//row = mysql_fetch_row(result);
//			if(row <= 0)
//			{
//				break;
//			}
			// mysql_num_fields()返回結果集中的字段數
			dataItem.clear();
			for(unsigned int j = 0; j < mysql_num_fields(result); ++j)
			{
				//printf("[CMysqlHandler] Query Result: %s : %s\n", fields[j].name, row[j]);
				strField = fields[j].name;
				if(0 != row[j])
				{
					strItem = row[j];
				}
				else
				{
					strItem = "";
				}
				dataItem[strField] = strItem;
			}
			listRest.push_back(dataItem);
		}
		// 釋放結果集的內存
		mysql_free_result(result);
	}
	return TRUE;
}

int CMysqlHandler::getFields(std::string strTableName, std::set<std::string> &sFields)
{
	string strSQL;

	if( NULL == mpMySQL)
	{
		setError("MySQL Connector Invalid");
		return FALSE;
	}

	// mysql_query()執行成功返回0，失敗返回非0值。
	strSQL = "SELECT * FROM " + strTableName + " LIMIT 1";
	if(mysql_real_query(mpMySQL, strSQL.c_str(), strSQL.length()))
	{
		setError("Query Error");
		return FALSE;
	}
	else
	{
		MYSQL_RES *result = mysql_use_result(mpMySQL); // 獲取結果集
		if(NULL == result)
		{
			setError("MySQL Result Fail");
			return FALSE;
		}

		MYSQL_FIELD *fields = mysql_fetch_fields(result); // 獲取欄位
		MYSQL_ROW row;

		for(unsigned int j = 0; j < mysql_num_fields(result); ++j)
		{
			sFields.insert(fields[j].name);
		}
		// 釋放結果集的內存
		mysql_free_result(result);
	}
	return TRUE;
}

