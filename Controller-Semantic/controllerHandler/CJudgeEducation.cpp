/*
 * CJudgeEducation.cpp
 *
 *  Created on: 2017年6月7日
 *      Author: Jugo
 */

#include <string>
#include "CJudgeEducation.h"
#include "JSONObject.h"
#include "config.h"
#include "dictionary.h"
#include "common.h"
#include "CFileHandler.h"
#include "utility.h"

using namespace std;

CJudgeEducation::CJudgeEducation()
{
	loadEducationDictionary();
}

CJudgeEducation::~CJudgeEducation()
{

}

string CJudgeEducation::toString()
{
	return "CJudgeEducation";
}

int CJudgeEducation::word(const char *szInput, JSONObject* jsonResp)
{
	int nRand;
	string strWord;

	strWord = szInput;
	if(strWord.empty())
		return FALSE;

	jsonResp->put("type", TYPE_RESP_TTS);

	for(map<string, string>::iterator iter = mapEducation.begin(); mapEducation.end() != iter; ++iter)
	{
		if(string::npos != strWord.find(iter->first))
		{
			jsonResp->put("tts", iter->second);
			return TRUE;
		}
	}

	for(map<string, string>::iterator iter = mapEducationPoetry.begin(); mapEducationPoetry.end() != iter; ++iter)
	{
		if(string::npos != strWord.find(iter->first))
		{
			jsonResp->put("tts", iter->second);
			return TRUE;
		}
	}

	if(string::npos != strWord.find("唐詩"))
	{
		// random
		nRand = getRand(0, mapEducationPoetry.size() - 1);
		map<string, string>::iterator iter = mapEducationPoetry.begin();
		for(int i = 0; i < nRand; ++i)
			++iter;

		jsonResp->put("tts", iter->second);
		return TRUE;
	}

	return TRUE;
}

int CJudgeEducation::evaluate(const char *szWord)
{
	int nScore;
	string strWord;

	nScore = 0;
	strWord = szWord;

	if(strWord.empty())
		return 0;

	//======== 評估關鍵字 ==========//
	if(string::npos != strWord.find("唐詩"))
		++nScore;

	//======== 評估字典檔 ==========//
	for(map<string, string>::iterator iter = mapEducation.begin(); mapEducation.end() != iter; ++iter)
	{
		if(string::npos != strWord.find(iter->first))
		{
			++nScore;
			break;
		}
	}

	for(map<string, string>::iterator iter = mapEducationPoetry.begin(); mapEducationPoetry.end() != iter; ++iter)
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
			_log("[CJudgeEducation] evaluate VERB: %s", wordAttr.strWord.c_str());
		}
	}

	return nScore;
}

void CJudgeEducation::loadEducationDictionary()
{
	int nIndex;
	CFileHandler fh;
	set<string> setData;
	set<string>::const_iterator iter;
	string strKey;
	string strValue;
	string strWord;

	setData.clear();
	fh.readAllLine("dictionary/match_education.txt", setData);
	for(iter = setData.begin(); setData.end() != iter; ++iter)
	{
		if(!iter->empty())
		{
			strWord = trim(*iter);
			if(!strWord.empty() && (0 < strWord.length()))
			{
				nIndex = iter->find(",");
				strKey = iter->substr(0, nIndex);
				strValue = iter->substr(nIndex + 1);
				mapEducation[strKey] = strValue;
			}
		}
	}
	_log("[CJudgeEducation] loadEducationDictionary Load Education: %d", mapEducation.size());

	setData.clear();
	fh.readAllLine("dictionary/match_education_poetry.txt", setData);
	for(iter = setData.begin(); setData.end() != iter; ++iter)
	{
		if(!iter->empty())
		{
			strWord = trim(*iter);
			if(!strWord.empty() && (0 < strWord.length()))
			{
				nIndex = iter->find(",");
				strKey = iter->substr(0, nIndex);
				strValue = iter->substr(nIndex + 1);
				mapEducationPoetry[strKey] = strValue;
			}
		}
	}
	_log("[CJudgeEducation] loadEducationDictionary Load Education Poetry: %d", mapEducationPoetry.size());
}

