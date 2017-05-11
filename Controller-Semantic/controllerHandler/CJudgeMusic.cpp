/*
 * CJudgeMusic.cpp
 *
 *  Created on: 2017年5月11日
 *      Author: root
 */

#include <algorithm>
#include <string>
#include "CJudgeMusic.h"
#include "JSONObject.h"
#include "common.h"
#include "dic_artist.h"
#include "dic_semantic.h"
#include "config.h"

using namespace std;

CJudgeMusic::CJudgeMusic()
{

}

CJudgeMusic::~CJudgeMusic()
{

}

int CJudgeMusic::word(const char *szInput, JSONObject* jsonResp)
{
	return TRUE;
}

int CJudgeMusic::evaluate(const char *szWord)
{
	int nScore;
	string strWord;
	string strValue;

	map<string, string>::const_iterator it_map;
	set<string>::const_iterator it_set;

	nScore = 0;
	strWord = szWord;

	if(strWord.empty())
		return 0;

	//======== 評估關鍵字 ==========//
	if(string::npos != strWord.find("歌"))
		++nScore;

	//======== 評估中文歌手字典檔 ==========//
	for(it_set = setArtistTaiwan.begin(); setArtistTaiwan.end() != it_set; ++it_set)
	{
		strValue = *it_set;
		transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
		_log("[CJudgeMusic] evaluate Artist Taiwan: %s", strValue.c_str());
	}

	//======== 評估字典檔 ==========//

	//======== 評估動詞 ===========//
	WORD_ATTR wordAttr;
	if(0 <= getVerb(strWord.c_str(), wordAttr))
	{
		if(VERB_LISTEN == wordAttr.nSubAttr)
		{
			++nScore;
			_log("[CJudgeMusic] evaluate VERB: %s", wordAttr.strWord.c_str());
		}
	}

	return nScore;
}

