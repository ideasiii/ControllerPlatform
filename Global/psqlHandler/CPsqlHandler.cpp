/*
 * CPsqlHandler.cpp
 *
 *  Created on: 2017年1月5日
 *      Author: jugo
 */

#include "CPsqlHandler.h"
#include <libpq-fe.h>

PGconn *conn = 0;

CPsqlHandler::CPsqlHandler()
{
	conn = 0;
}

CPsqlHandler::~CPsqlHandler()
{
	close();
}

int CPsqlHandler::open(const char *pghost, const char *pgport, const char *dbName, const char *login, const char *pwd)
{
	conn = PQsetdbLogin(pghost, pgport, NULL, NULL, dbName, login, pwd);

	//ConnStatusType的值最常用的两个是CONNECTION_OK或 CONNECTION_BAD。
	if (PQstatus(conn) != CONNECTION_OK)
	{
		printf("[CPsqlHandler] Connection to database failed: %s", PQerrorMessage(conn));
		close();
		return 0;
	}

	return 1;
}

void CPsqlHandler::close()
{
	if (0 != conn)
		PQfinish(conn);
	conn = 0;
}

int CPsqlHandler::sqlExec(const char *szSQL)
{
	int nRet = ERROR_SUCCESS;

	if (0 == conn)
	{
		printf("[CPsqlHandler] Invalid PGconn");
		return ERROR_INVALID_CONN;
	}
	PGresult *res = PQexec(conn, szSQL);

	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		printf("[CPsqlHandler] sqlExec Fail: %s", szSQL);
		nRet = ERROR_FAIL_EXECSQL;
	}
	PQclear(res);
	return nRet;
}

int CPsqlHandler::sqlExec(list<string> listSQL)
{
	if (0 == conn)
	{
		printf("[CPsqlHandler] Invalid PGconn");
		return ERROR_INVALID_CONN;
	}

	PGresult *res = PQexec(conn, "BEGIN");

	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		printf("BEGIN command failed\n");
		PQclear(res);
		return ERROR_FAIL_BEGIN;
	}

	string strItem;
	for (list<string>::iterator it = listSQL.begin(); it != listSQL.end(); ++it)
	{
		strItem = (*it);
		if (!strItem.empty())
		{
			res = PQexec(conn, strItem.c_str());

			if (PQresultStatus(res) != PGRES_COMMAND_OK)
			{
				printf("SQL Exec failed: %s\n", PQerrorMessage(conn));
				PQclear(res);
			}
		}
	}

	res = PQexec(conn, "COMMIT");

	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		printf("COMMIT command failed\n");
		PQclear(res);
		return ERROR_FAIL_COMMIT;
	}

	PQclear(res);

	return ERROR_SUCCESS;
}

int CPsqlHandler::query(const char *szSQL, list<map<string, string> > &listRest)
{
	if (0 == conn)
	{
		printf("[CPsqlHandler] Invalid PGconn");
		return ERROR_INVALID_CONN;
	}

	PGresult *res = PQexec(conn, szSQL);

	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{

		printf("[CPsqlHandler] No data retrieved :%s\n", szSQL);
		PQclear(res);
		return 0;
	}

	int rows = PQntuples(res);

	if (0 < rows)
	{
		map<string, string> mapValue;
		int nFields = PQnfields(res);
		for (int i = 0; i < PQntuples(res); ++i)
		{
			mapValue.clear();
			for (int j = 0; j < nFields; ++j)
			{
				mapValue[PQfname(res, j)] = PQgetvalue(res, i, j);
			}
			listRest.push_back(mapValue);
		}
		mapValue.clear();
	}

	PQclear(res);
	return listRest.size();
}
