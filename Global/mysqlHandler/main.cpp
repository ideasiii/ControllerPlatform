/*
 * main.cpp
 *
 *  Created on: 2017年01月05日
 *      Author: Jugo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "CMysqlHandler.h"

using namespace std;

int main()
{
	CMysqlHandler *pmysql = new CMysqlHandler();

	pmysql->connect("124.9.6.64", "tracker", "mobile_tracker", "tracker123!");
	pmysql->close();


	pmysql->connect("124.9.6.64", "tracker", "mobile_tracker", "tracker123!");
	pmysql->close();

	delete pmysql;
	return 0;
}
