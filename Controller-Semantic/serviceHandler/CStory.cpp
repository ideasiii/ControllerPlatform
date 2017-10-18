/*
 * CStory.cpp
 *
 *  Created on: 2017年10月17日
 *      Author: jugo
 */

#include <map>
#include <set>
#include <list>
#include "CStory.h"
#include "CResponsePacket.h"
#include "LogHandler.h"
#include "CMysqlHandler.h"
#include "config.h"

using namespace std;

set<CString> setMaterial;

CStory::CStory() :
		mysql(0)
{
	mysql = new CMysqlHandler();
}

CStory::~CStory()
{
	if(mysql->isValid())
		mysql->close();
	delete mysql;
}

void CStory::init()
{
	CString strSQL;
	list<map<string, string> > listValue;

	if(mysql->connect(EDUBOT_HOST, EDUBOT_DB, EDUBOT_ACCOUNT, EDUBOT_PASSWD, "5"))
	{
		strSQL = "SELECT material FROM edubot.story_material";
		mysql->query(strSQL.toString(), listValue);
	}
}

int CStory::evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch)
{
	CString strSQL;
	int nScore;

	nScore = 0;
	strSQL = "SELECT * FROM";

	return nScore;
}

int CStory::activity(const char *szInput, JSONObject& jsonResp)
{

	return 0;
}

CString CStory::name()
{
	return "Story Service";
}

