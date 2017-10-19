/*
 * CStory.cpp
 *
 *  Created on: 2017年10月17日
 *      Author: jugo
 */

#include <map>

#include "CStory.h"
#include "CResponsePacket.h"
#include "LogHandler.h"
#include "CMysqlHandler.h"
#include "config.h"
#include "CString.h"
#include "utility.h"

using namespace std;

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
	string strName;
	string strMaterial;
	CString strSQL;
	list<map<string, string> > listValue;
	list<map<string, string> >::iterator it_list;
	map<string, string> mapItem;

	if(mysql->connect(EDUBOT_HOST, EDUBOT_DB, EDUBOT_ACCOUNT, EDUBOT_PASSWD, "5"))
	{
//		strSQL = "SELECT material FROM edubot.story_material";
//		if(mysql->query(strSQL.toString(), listValue))
//		{
//			listMaterial.clear();
//			for(it_list = listValue.begin(); listValue.end() != it_list; ++it_list)
//			{
//				mapItem.clear();
//				mapItem = *it_list;
//				for(map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
//				{
//					listMaterial.push_back((*j).second);
//				}
//			}
//
//			_log("[CStory] init Load story_material count: %d", listMaterial.size());
//		for(list<CString>::iterator it = listMaterial.begin(); listMaterial.end() != it; ++it)
//		{
//			printf("%s\n", (*it).getBuffer());
//		}
//		}

		listValue.clear();
		strSQL = "SELECT name,material FROM edubot.story";
		if(mysql->query(strSQL.toString(), listValue))
		{
			mapStoryMaterial.clear();
			for(it_list = listValue.begin(); listValue.end() != it_list; ++it_list)
			{
				mapItem.clear();
				mapItem = *it_list;
				for(map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
				{
					if(!j->first.compare("name"))
						strName = j->second;
					if(!j->first.compare("material"))
						strMaterial = j->second;
				}
				mapStoryMaterial.insert(pair<string, string>(strName, strMaterial));
				spliteData(const_cast<char*>(strMaterial.c_str()), ",", setMaterial);
			}
			_log("[CStory] init Load story count: %d", mapStoryMaterial.size());
			for(map<string, string>::iterator it = mapStoryMaterial.begin(); mapStoryMaterial.end() != it; ++it)
			{
				printf("%s - %s\n", (*it).first.c_str(), (*it).second.c_str());
			}
			for(set<string>::iterator it_set = setMaterial.begin(); setMaterial.end() != it_set; ++it_set)
			{
				printf("%s\n", it_set->c_str());
			}
		}
	}
}

int CStory::evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch)
{
	map<int, string> mapStory;
	list<map<string, string> > listValue;
	list<map<string, string> >::iterator it_list;
	map<string, string>::iterator it_map;
	CString strWord;
	CString strMaterial;
	int nScore;
	int nIndex;
	int nRand;

	nScore = 0;
	strWord = szWord;
	nIndex = 0;

	for(set<string>::iterator it_set = setMaterial.begin(); setMaterial.end() != it_set; ++it_set)
	{
		strWord.trim();
		if(-1 != strWord.find(trim(*it_set).c_str()))
		{
			for(it_map = mapStoryMaterial.begin(); mapStoryMaterial.end() != it_map; ++it_map)
			{
				if(string::npos != it_map->second.find(trim(*it_set)))
				{
					_log("[CStory] evaluate find story: %s <-- %s", it_map->first.c_str(), it_set->c_str());
					mapStory[nIndex++] = it_map->first;
				}
			}
		}
	}

	if(mapStory.size())
	{
		nRand = getRand(0, mapStory.size() - 1);
		mapMatch["dictionary"] = mapStory[nRand];
		++nScore;
		_log("[CStory] evaluate rand index: %d story: %s", nRand, mapMatch["dictionary"].c_str());
	}
	return nScore;
}

int CStory::activity(const char *szInput, JSONObject& jsonResp)
{

	return 0;
}

CString CStory::name()
{
	return "Story Search by Material Service";
}

