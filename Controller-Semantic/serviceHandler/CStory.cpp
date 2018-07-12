/*
 * CStory.cpp
 *
 *  Created on: 2017年10月17日
 *      Author: jugo
 */

#include <map>
#include <vector>
#include "CStory.h"
#include "CResponsePacket.h"
#include "LogHandler.h"
#include "CMysqlHandler.h"
#include "config.h"
#include "CString.h"
#include "utility.h"
#include "CRankingHandler.cpp"
#include "CFileHandler.h"
#include "utility.h"

#define STORY_FILE_PATH			"/data/opt/tomcat/webapps/story/"

using namespace std;

CStory::CStory()
{

}

CStory::~CStory()
{

}

void CStory::init()
{
	string strName;
	string strMaterial;
	string strMaterialTitle;
	CString strSQL;
	list<map<string, string> > listValue;
	list<map<string, string> >::iterator it_list;
	map<string, string> mapItem;
	CMysqlHandler *mysql;

	mysql = new CMysqlHandler();

	if(mysql->connect(EDUBOT_HOST, EDUBOT_DB, EDUBOT_ACCOUNT, EDUBOT_PASSWD, "5"))
	{
		listValue.clear();
		strSQL = "SELECT name,material,material_title FROM edubot.story";
		if(mysql->query(strSQL.toString(), listValue))
		{
			mapStoryMaterial.clear();
			mapStoryMaterialTitle.clear();
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
					if(!j->first.compare("material_title"))
						strMaterialTitle = j->second;
				}
				mapStoryMaterial.insert(pair<string, string>(strName, strMaterial));
				mapStoryMaterialTitle.insert(pair<string, string>(strName, strMaterialTitle));
				spliteData(const_cast<char*>(strMaterial.c_str()), ",", setMaterial);
			}
			_log("[CStory] init Load story count: %d", mapStoryMaterial.size());
//			for(map<string, string>::iterator it = mapStoryMaterial.begin(); mapStoryMaterial.end() != it; ++it)
//			{
//				printf("%s - %s\n", (*it).first.c_str(), (*it).second.c_str());
//			}
//			for(set<string>::iterator it_set = setMaterial.begin(); setMaterial.end() != it_set; ++it_set)
//			{
//				printf("%s\n", it_set->c_str());
//			}
		}
	}

	if(mysql->isValid())
		mysql->close();
	delete mysql;
}

int CStory::evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch)
{
	CRankingHandler<string, int> ranking;
	map<int, string> mapStory;
	list<map<string, string> > listValue;
	list<map<string, string> >::iterator it_list;
	map<string, string>::iterator it_map;
	CString strWord;
	CString strMaterial;
	vector<string> listTop;
	int nScore;
	int nIndex;
	int nValue;

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
					_log("[CStory] evaluate find material in story content: %s <-- %s", it_map->first.c_str(),
							it_set->c_str());
					mapStory[nIndex++] = it_map->first;
					nValue = ranking.getValue(it_map->first, 0);
					// find in story title
					if(string::npos != it_map->first.find(*it_set))
					{
						nValue += 3;
						_log("[CStory] evaluate find material in story title: %s <-- %s", it_map->first.c_str(),
								it_set->c_str());
					}
					ranking.add(it_map->first, ++nValue);
					_log("[CStory] evaluate ranking get %s value: %d", it_map->first.c_str(), nValue);
				}
			}
		}
	}

	if(ranking.size())
	{
		ranking.topValueKeys(listTop);
		mapMatch["dictionary"] = listTop.at(getRand(0, listTop.size() - 1));
		++nScore;
		_log("[CStory] evaluate Top Score: %d story: %s", ranking.topValue(), mapMatch["dictionary"].c_str());
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

void CStory::storyAnalysis()
{
	int nIndex;
	CFileHandler fh;
	set<string> setData;
	set<string>::const_iterator iter_set;
	string strFileName;
	CString strFilePath;
	string strContent;
	set<string> setDictionary;
	CMysqlHandler mysql;
	CString strMaterial;
	CString strMaterialTitle;
	CString strSQL;

	fh.readAllLine("dictionary/story_material.txt", setDictionary);
	fh.readAllLine("dictionary/animal.txt", setDictionary);
	fh.readAllLine("dictionary/affect.txt", setDictionary);
	//fh.readAllLine("dictionary/whatever.txt", setDictionary);

	fh.readPath(STORY_FILE_PATH, setData);

	mysql.connect(EDUBOT_HOST, EDUBOT_DB, EDUBOT_ACCOUNT, EDUBOT_PASSWD, "5");

	for(iter_set = setData.begin(); setData.end() != iter_set; ++iter_set)
	{
		nIndex = iter_set->rfind(".");
		if((int) string::npos != nIndex)
		{
			if(!iter_set->substr(nIndex + 1).compare("txt"))
			{
				strFileName = trim(iter_set->substr(0, nIndex));
				_log("[CStory] storyAnalysis Start analysis story: %s", iter_set->c_str());
				strFilePath.format("%s%s", STORY_FILE_PATH, iter_set->c_str());
				fh.readContent(strFilePath.getBuffer(), strContent);
				if(!strContent.empty())
				{
					//_log("[CStory] storyAnalysis Start analysis story content: %s", strContent.c_str());
					strMaterial = "";
					strMaterialTitle = "";

					for(set<string>::iterator it_set = setDictionary.begin(); setDictionary.end() != it_set; ++it_set)
					{
						// 故事檔名分析比對
						if(string::npos != strFileName.find(trim(*it_set)))
						{
							if(strMaterialTitle.Compare(""))
								strMaterialTitle += ",";
							strMaterialTitle = strMaterialTitle + *it_set;
						}

						// 故事內容分析比對
						if(string::npos != strContent.find(trim(*it_set)))
						{
							//_log("[CStory] storyAnalysis find material: %s", it_set->c_str());
							if(strMaterial.Compare(""))
								strMaterial += ",";
							strMaterial = strMaterial + *it_set;

						}
					}
					_log("[CStory] storyAnalysis find material: %s", strMaterial.getBuffer());
					_log("[CStory] storyAnalysis find material title: %s", strMaterialTitle.getBuffer());
					if(strMaterial.getLength() || strMaterialTitle.getLength())
					{
						strSQL.format("DELETE FROM story WHERE name = '%s'", strFileName.c_str());
						mysql.sqlExec(strSQL.toString());

						strSQL.format("INSERT INTO story(name,material,material_title) VALUES('%s','%s','%s')",
								strFileName.c_str(), strMaterial.getBuffer(), strMaterialTitle.getBuffer());
						mysql.sqlExec(strSQL.toString());
					}
				}
			}
		}
	}

	if(mysql.isValid())
		mysql.close();
}

