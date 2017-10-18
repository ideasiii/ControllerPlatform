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
#include "CString.h"

using namespace std;

list<CString> listMaterial;

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
	list<map<string, string> >::iterator it_list;
	map<string, string> mapItem;

	if(mysql->connect(EDUBOT_HOST, EDUBOT_DB, EDUBOT_ACCOUNT, EDUBOT_PASSWD, "5"))
	{
		strSQL = "SELECT material FROM edubot.story_material";
		if(mysql->query(strSQL.toString(), listValue))
		{
			listMaterial.clear();
			for(it_list = listValue.begin(); listValue.end() != it_list; ++it_list)
			{
				mapItem.clear();
				mapItem = *it_list;
				for(map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
				{
					listMaterial.push_back((*j).second);
				}
			}

			_log("[CStory] init Load story material count: %d", listMaterial.size());
//		for(list<CString>::iterator it = listMaterial.begin(); listMaterial.end() != it; ++it)
//		{
//			printf("%s\n", (*it).getBuffer());
//		}
		}
	}
}

int CStory::evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch)
{
	map<string, string> mapItem;
	list<map<string, string> > listValue;
	list<map<string, string> >::iterator it_list;
	CString strSQL;
	CString strWord;
	int nScore;

	nScore = 0;
	strWord = szWord;

	for(list<CString>::iterator it = listMaterial.begin(); listMaterial.end() != it; ++it)
	{
		strWord.makeLower().trim();
		if(-1 != strWord.find(it->makeLower().trim().getBuffer()))
		{
			_log("[CStory] evaluate word: %s find: %s", strWord.getBuffer(), (*it).getBuffer());
			strSQL.format("SELECT name FROM edubot.story where material LIKE '%%%s%%';", (*it).getBuffer());
			if(mysql->isValid())
			{
				if(mysql->query(strSQL.toString(), listValue))
				{
					for(it_list = listValue.begin(); listValue.end() != it_list; ++it_list)
					{
						mapItem.clear();
						mapItem = *it_list;
						for(map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
						{
							mapMatch["dictionary"] = (*j).second;
							return 1;
						}
					}
				}
			}
			else
			{
				_log("[CStory] evaluate mysql invalid");
			}
		}

		//_log("find: %s", it->getBuffer());
	}

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

