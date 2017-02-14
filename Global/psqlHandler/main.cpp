/*
 * main.cpp
 *
 *  Created on: 2017年01月05日
 *      Author: Jugo
 */

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string>
#include <map>
#include <list>

#include "CPsqlHandler.h"

using namespace std;

void do_exit(PGconn *conn)
{

	PQfinish(conn);
	exit(1);
}

void handler()
{
	CPsqlHandler *psql = new CPsqlHandler();
	if (psql->open("175.98.119.121", "5432", "tracker", "tracker", "ideas123!"))
	{
		printf("Connect Postgresql Success\n");
		time_t now = time(0);
		struct tm tstruct;
		char buf[80];
		tstruct = *localtime(&now);
		strftime(buf, sizeof(buf), "%Y%m%d%X", &tstruct);

		// insert record
		string strSQL = "INSERT INTO tracker_user (id,app_id)VALUES('" + string(buf) + "','testhandler')";
		psql->sqlExec(strSQL.c_str());
		psql->sqlExec(strSQL.c_str());

		strSQL = "INSERT INTO tracker_user (id,app_id)VALUES('" + string(buf) + "xxxx" + "','testhandler')";
		psql->sqlExec(strSQL.c_str());

		// insert record use trans
		list<string> listSQL;
		strSQL = "INSERT INTO tracker_user (id,app_id)VALUES('" + string(buf) + "2" + "','testhandlerttttt1')";
		listSQL.push_back(strSQL);
		strSQL = "INSERT INTO tracker_user (id,app_id)VALUES('" + string(buf) + "3" + "','testhandlerttttt22222')";
		listSQL.push_back(strSQL);
		strSQL = "INSERT INTO tracker_user (id,app_id)VALUES('" + string(buf) + "2" + "','testhandlerttttt1')";
		listSQL.push_back(strSQL);

		psql->sqlExec(listSQL);

		// query db
		strSQL = "SELECT * FROM tracker_user";
		list<map<string, string> > listRest;
		psql->query(strSQL.c_str(), listRest);

		string strField;
		string strValue;
		map<string, string> mapItem;
		int nCount = 0;
		for (list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i, ++nCount)
		{
			mapItem = *i;
			for (map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
			{
				printf("%s : %s\n", (*j).first.c_str(), (*j).second.c_str());
			}

		}
		printf("=============================%d================================\n", nCount);

		psql->close();
	}
	delete psql;
}

int main()
{
	handler();
	return 0;
	//char *conninfo = "hostaddr=175.98.119.121 port=5432 dbname=tracker user=tracker password=ideas123!";
	//PGconn *conn = PQconnectdb(conninfo);
	PGconn *conn = PQsetdbLogin("175.98.119.121", "5432", NULL, NULL, "tracker", "tracker", "ideas123!");

	//ConnStatusType的值最常用的两个是CONNECTION_OK或 CONNECTION_BAD。
	if (PQstatus(conn) != CONNECTION_OK)
	{

		fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
		do_exit(conn);
	}

	printf("User: %s\n", PQuser(conn));
	printf("Database name: %s\n", PQdb(conn));
	printf("Password: %s\n", PQpass(conn));

	// PQparameterStatus
	//
	printf("server_version: %s\n", PQparameterStatus(conn, "server_version"));
	printf("server_encoding: %s\n", PQparameterStatus(conn, "server_encoding"));
	printf("client_encoding: %s\n", PQparameterStatus(conn, "client_encoding"));
	printf("session_authorization: %s\n", PQparameterStatus(conn, "session_authorization"));
	printf("DateStyle: %s\n", PQparameterStatus(conn, "DateStyle"));
	printf("TimeZone: %s\n", PQparameterStatus(conn, "TimeZone"));
	printf("integer_datetimes: %s\n", PQparameterStatus(conn, "integer_datetimes"));
	printf("standard_conforming_strings: %s\n", PQparameterStatus(conn, "standard_conforming_strings"));

	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y%m%d%X", &tstruct);

	string strSQL = "INSERT INTO tracker_user (id,app_id)VALUES('" + string(buf) + "','test')";

	PGresult *res = PQexec(conn, strSQL.c_str());

	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		printf("db insert fail\n");
		do_exit(conn);
	}

	res = PQexec(conn, "SELECT * FROM tracker_user");

	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{

		printf("No data retrieved\n");
		PQclear(res);
		do_exit(conn);
	}

	int rows = PQntuples(res);

	for (int i = 0; i < rows; i++)
	{

		printf("%s %s %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1), PQgetvalue(res, i, 11));
	}

	PQclear(res);
	PQfinish(conn);

	return 0;
}
