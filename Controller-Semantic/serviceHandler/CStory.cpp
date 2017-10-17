/*
 * CStory.cpp
 *
 *  Created on: 2017年10月17日
 *      Author: jugo
 */

#include "CStory.h"
#include "CResponsePacket.h"
#include "LogHandler.h"
#include "CMysqlHandler.h"
#include "config.h"

CStory::CStory()
{

}

CStory::~CStory()
{

}

void CStory::init()
{

}

int CStory::evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch)
{
	CString strSQL;
	int nScore;

	nScore = 0;

	CMysqlHandler *pmysql = new CMysqlHandler();
	pmysql->connect(EDUBOT_HOST, EDUBOT_DB, EDUBOT_ACCOUNT, EDUBOT_PASSWD, "5");

	strSQL = "SELECT * FROM";
	pmysql->close();
	delete pmysql;
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

