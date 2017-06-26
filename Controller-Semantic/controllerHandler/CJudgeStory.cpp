/*
 * CJudgeStory.cpp
 *
 *  Created on: 2017年5月2日
 *      Author: Jugo
 */

#include <set>
#include <map>
#include <string>
#include "CJudgeStory.h"
#include "config.h"
#include "dictionary.h"
#include "common.h"
#include "CFileHandler.h"
#include "CResponsePacket.h"
#include "JSONObject.h"
#include "JSONArray.h"

using namespace std;

CJudgeStory::CJudgeStory()
{
	loadStoryDictionary();
}

CJudgeStory::~CJudgeStory()
{

}

std::string CJudgeStory::toString()
{
	return "CJudgeStory";
}

int CJudgeStory::word(const char *szInput, JSONObject& jsonResp, map<string, string> &mapMatch)
{
	CResponsePacket respPacket;
	string strWord;
	string strFile;
	string strStory;

	strWord = szInput;
	if(strWord.empty())
		return FALSE;

	for(map<string, string>::iterator iter = mapStory.begin(); mapStory.end() != iter; ++iter)
	{
		if(string::npos != strWord.find(iter->first))
		{
			strFile = iter->second;
			strStory = iter->first;
			break;
		}
	}

	if(strFile.empty())
	{
		strFile = "三隻小豬.mp3";
		strStory = "三隻小豬";
	}

	respPacket.setData("host", STORY_HOST).setData("file", strFile).setData("story", strStory).format(TYPE_RESP_STORY,
			jsonResp);
	return TRUE;
}

int CJudgeStory::evaluate(const char *szWord, map<string, string> &mapMatch)
{
	int nScore;
	string strWord;

	nScore = 0;
	strWord = szWord;

	if(strWord.empty())
		return 0;

	//======== 評估關鍵字 ==========//
	if(string::npos != strWord.find("故事") || string::npos != strWord.find("story"))
		++nScore;

	//======== 評估字典檔 ==========//
	for(map<string, string>::iterator iter = mapStory.begin(); mapStory.end() != iter; ++iter)
	{
		if(string::npos != strWord.find(iter->first))
		{
			++nScore;
			break;
		}
	}

	//======== 評估動詞 ===========//
	WORD_ATTR wordAttr;
	if(0 <= getVerb(strWord.c_str(), wordAttr))
	{
		if(VERB_LISTEN == wordAttr.nSubAttr)
		{
			++nScore;
			_log("[CJudgeStory] evaluate VERB: %s", wordAttr.strWord.c_str());
		}
	}

	return nScore;
}

void CJudgeStory::loadStoryDictionary()
{
	int nIndex;
	CFileHandler fh;
	set<string> setData;
	set<string> setMood;
	set<string>::const_iterator iter;
	set<string>::const_iterator iter_set_mood;
	string strFileType;
	string strWord;
	string strFile;
	string strFileName;
	string strPathMood;
	string strTime;
	string strMood;
	JSONArray jArrayMood;
	JSONObject jobjMood;

	fh.readPath(STORY_PATH, setData);
	mapStoryMood.clear();

	for(iter = setData.begin(); setData.end() != iter; ++iter)
	{
		nIndex = iter->rfind(".");
		if((int) string::npos != nIndex)
		{
			strFileType = iter->substr(nIndex + 1);
			if(!strFileType.compare("mp3") || !strFileType.compare("MP3"))
			{
				strFileName = iter->substr(0, nIndex);
				mapStory[strFileName] = iter->c_str();
				setMood.clear();
				strPathMood = PATH_STORY_MOOD + iter->substr(0, nIndex) + ".mood";
				if(0 < fh.readAllLine(strPathMood.c_str(), setMood))
				{
					jArrayMood.create();
					for(iter_set_mood = setMood.begin(); setMood.end() != iter_set_mood; ++iter_set_mood)
					{
						if(!iter_set_mood->empty())
						{
							nIndex = iter_set_mood->find(",");
							strTime = iter_set_mood->substr(0, nIndex);
							strMood = iter_set_mood->substr(nIndex + 1);
							jobjMood.create();
							jobjMood.put("time", strTime);
							jobjMood.put("host", PATH_STORY_MOOD);
							jobjMood.put("file", strMood);
							jobjMood.put("description", strMood);
							jArrayMood.add(jobjMood);
							jobjMood.release();
						}
					}
					mapStoryMood[strFileName] = jArrayMood;
					jArrayMood.release();
				}
			}
		}
	}

	setData.clear();
	fh.readAllLine("dictionary/match_file_story.txt", setData);
	for(iter = setData.begin(); setData.end() != iter; ++iter)
	{
		if(!iter->empty())
		{
			nIndex = iter->find(",");
			strWord = iter->substr(0, nIndex);
			strFile = iter->substr(nIndex + 1);
			mapStory[strWord] = strFile;
		}
	}

	_log("[CJudgeStory] loadStoryDictionary Load Story: %d", mapStory.size());
//	for(map<string, string>::const_iterator it = mapStory.begin(); mapStory.end() != it; ++it)
//		_log("%s - %s", it->first.c_str(), it->second.c_str());

}

