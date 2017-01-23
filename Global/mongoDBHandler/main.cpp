/*
 * main.cpp
 *
 *  Created on: 2015年12月10日
 *      Author: Louis Ju
 */

#include "CMongoDBHandler.h"
#include "common.h"
#include <list>
#include <string>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
	CMongoDBHandler *mongo = CMongoDBHandler::getInstance();
	mongo->connectDB("127.0.0.1", "27017");
	list<string> listJSON;
	mongo->query("access", "mobile", "ID", "826BB4DE-A54E-42E8-BD9C-DFE15CF2F4EB1472188091474", listJSON);

	delete mongo;

	string strJSON;
	for (list<string>::iterator i = listJSON.begin(); i != listJSON.end(); ++i)
	{
		strJSON = *i;
		cout << strJSON << endl;
	}

}

