/*
 * CJudgeMusic.cpp
 *
 *  Created on: 2017年5月11日
 *      Author: root
 */

#include <algorithm>
#include <string>
#include <set>
#include <map>
#include "CJudgeMusic.h"
#include "JSONObject.h"
#include "common.h"
#include "dic_artist.h"
#include "dic_semantic.h"
#include "dic_music_artist_english_female.h"
#include "config.h"
#include "CSpotify.h"

using namespace std;

CJudgeMusic::CJudgeMusic()

{
	setpFnGetArtist.insert(&CJudgeMusic::getArtistTaiwan);
	setpFnGetArtist.insert(&CJudgeMusic::getArtistEnglish);
	setpFnGetArtist.insert(&CJudgeMusic::getArtistEnglishFemale);
	setpFnGetArtist.insert(&CJudgeMusic::getArtistEnglishMale);
}

CJudgeMusic::~CJudgeMusic()
{

}

int CJudgeMusic::word(const char *szInput, JSONObject* jsonResp)
{
	string strArtist;
	string strAlbum;
	string strTrack;
	string strTrackUri;

	(this->*this->setpFnGetArtist[0])(szInput);

//	for(set<pFnGetArtist>::const_iterator iter = setpFnGetArtist.begin(); setpFnGetArtist.end() != iter; ++iter)
//	{
//		strArtist = (*iter)(szInput);
//		if(!strArtist.empty())
//			break;
//	}

//	strArtist = getArtistTaiwan(szInput);
//	if(strArtist.empty())
//		strArtist = getArtistEnglish(szInput);
//
//	if(strArtist.empty())
//		strArtist = getArtistEnglishFemale(szInput);

	if(!strArtist.empty())
	{
		CSpotify spotify;
		map<string, string> mapAlbums;
		map<string, string>::const_iterator it;
		if(spotify.getAlbum(strArtist.c_str(), mapAlbums, "TW"))
		{
			for(it = mapAlbums.begin(); mapAlbums.end() != it; ++it)
			{
				_log("[CJudgeMusic] word get %s - %s -- %s", strArtist.c_str(), it->first.c_str(), it->second.c_str());
				map<int, TRACK> mapSong;
				spotify.getTrack(it->second.c_str(), mapSong, "TW");
				for(map<int, TRACK>::const_iterator cit = mapSong.begin(); mapSong.end() != cit; ++cit)
				{
					_log("			 %s - %s", it->first.c_str(), cit->second.name.c_str());
					if(strTrack.empty())
					{
						strAlbum = it->first;
						strTrack = cit->second.name;
						strTrackUri = cit->second.uri;
					}
				}
			}
		}
	}

	if(!strTrackUri.empty())
	{
		JSONObject jsonSpotify;
		jsonSpotify.put("source", 2);
		jsonSpotify.put("album", strAlbum);
		jsonSpotify.put("artist", strArtist);
		jsonSpotify.put("song", strTrack);
		jsonSpotify.put("id", strTrackUri);
		jsonResp->put("type", TYPE_RESP_MUSIC);
		jsonResp->put("music", jsonSpotify);
	}

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
		transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);
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
		transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);
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

string CJudgeMusic::getArtistEnglishFemale(const char *szWord)
{
	string strWord;
	string strValue;
	string strArtist;
	set<string>::const_iterator it_set;

	strWord = szWord;

	if(!strWord.empty())
	{
		transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);
		for(it_set = setArtistEnglishFemale.begin(); setArtistEnglishFemale.end() != it_set; ++it_set)
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

string CJudgeMusic::getArtistEnglishMale(const char *szWord)
{
	string strWord;
	string strValue;
	string strArtist;
	set<string>::const_iterator it_set;

	strWord = szWord;

	if(!strWord.empty())
	{
		transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);
		for(it_set = setArtistEnglishFemale.begin(); setArtistEnglishFemale.end() != it_set; ++it_set)
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
