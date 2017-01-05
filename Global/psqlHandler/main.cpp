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

using namespace std;

void do_exit(PGconn *conn)
{

	PQfinish(conn);
	exit(1);
}

int main()
{

	//char *conninfo = "hostaddr=175.98.119.121 port=5432 dbname=tracker user=tracker password=ideas123!";
	//PGconn *conn = PQconnectdb(conninfo);
	PGconn *conn = PQsetdbLogin("175.98.119.121", "5432", NULL, NULL, "tracker", "tracker", "ideas123!");

	//ConnStatusType的值最常用的两个是CONNECTION_OK或 CONNECTION_BAD。
	if (PQstatus(conn) != CONNECTION_OK)
	{

		fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
		do_exit(conn);
	}

	char *user = PQuser(conn);
	char *db_name = PQdb(conn);
	char *pswd = PQpass(conn);

	printf("User: %s\n", user);
	printf("Database name: %s\n", db_name);
	printf("Password: %s\n", pswd);

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
