/*
 * CJudgeAbsolutely.cpp
 *
 *  Created on: 2017年6月6日
 *      Author: Jugo
 */
#include <set>
#include <string>
#include "CJudgeAbsolutely.h"
#include "CFileHandler.h"
#include "dictionary.h"
#include "common.h"

using namespace std;

CJudgeAbsolutely::CJudgeAbsolutely()
{
	loadAbsolutelyDictionary();
}

CJudgeAbsolutely::~CJudgeAbsolutely()
{

}

string CJudgeAbsolutely::toString()
{
	return "CJudgeAbsolutely";
}
int CJudgeAbsolutely::word(const char *szInput, JSONObject* jsonResp, map<string, string> &mapMatch)
{
	return 0;
}

int CJudgeAbsolutely::evaluate(const char *szWord, map<string, string> &mapMatch)
{
	return 0;
}

void CJudgeAbsolutely::loadAbsolutelyDictionary()
{
	int nIndex;
	int nCount;
	CFileHandler fh;
	set<string> setData;
	set<string>::const_iterator it_set;
	string strKey;
	string strValue;

	if((nCount = fh.readAllLine("dictionary/match_absolutely.txt", setData)))
	{
		for(it_set = setData.begin(); setData.end() != it_set; ++it_set)
		{
			if(!it_set->empty())
			{
				nIndex = it_set->find(",");
				strKey = it_set->substr(0, nIndex);
				strValue = it_set->substr(nIndex + 1);
				mapAbsolutly[strKey] = strValue;
			}
		}
	}
	_log("[CJudgeAbsolutely] loadAbsolutelyDictionary Total Absolutely Match Count: %d", mapAbsolutly.size());
}

