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
	string strArtist;

	strArtist = getArtistTaiwan(szInput);
	if(strArtist.empty())
		strArtist = getArtistEnglish(szInput);

	if(strArtist.empty())
		strArtist = "";

	JSONObject jsonSpotify;
	jsonSpotify.put("source", 2);
	jsonSpotify.put("album", "");
	jsonSpotify.put("artist", strArtist);
	jsonSpotify.put("song", "");
	jsonSpotify.put("id", "spotify:track:2TpxZ7JUBn3uw46aR7qd6V");
	jsonResp->put("type", TYPE_RESP_MUSIC);
	jsonResp->put("music", jsonSpotify);
	return TRUE;
}

int CJudgeMusic::evaluate(const char *szWord)
{
	int nScore;
	string strWord;
	string strArtist;

	map<string, string>::const_iterator it_map;
	set<string>::const_iterator it_set;

	nScore = 0;
	strWord = szWord;

	if(strWord.empty())
		return 0;

	transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);

	//======== 評估關鍵字 ==========//
	if(string::npos != strWord.find("歌"))
	{
		++nScore;
	}

	//======== 評估中文歌手字典檔 ==========//
	strArtist = getArtistTaiwan(szWord);
	if(!strArtist.empty())
		++nScore;

	//======== 評估英文歌手字典檔 ==========//
	strArtist = getArtistEnglish(szWord);
	if(!strArtist.empty())
		++nScore;

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

string CJudgeMusic::getArtistTaiwan(const char *szWord)
{
	string strWord;
	string strValue;
	string strArtist;
	set<string>::const_iterator it_set;

	strWord = szWord;
	if(!strWord.empty())
	{
		for(it_set = setArtistTaiwan.begin(); setArtistTaiwan.end() != it_set; ++it_set)
		{
			strValue = *it_set;
			transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
			if(string::npos != strWord.find(strValue))
			{
				strArtist = strValue;
				_log("[CJudgeMusic] evaluate Find Artist: %s", strValue.c_str());
				break;
			}
		}
	}

	return strArtist;
}

string CJudgeMusic::getArtistEnglish(const char *szWord)
{
	string strWord;
	string strValue;
	string strArtist;
	set<string>::const_iterator it_set;

	strWord = szWord;
	if(!strWord.empty())
	{
		for(it_set = setArtistEnglish.begin(); setArtistEnglish.end() != it_set; ++it_set)
		{
			strValue = *it_set;
			transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
			if(string::npos != strWord.find(strValue))
			{
				strArtist = strValue;
				_log("[CJudgeMusic] evaluate Find Artist: %s", strValue.c_str());
				break;
			}
		}
	}

	return strArtist;
}

