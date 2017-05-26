/*
 * CJudgeMusic.cpp
 *
 *  Created on: 2017年5月11日
 *      Author: root
 */

#include <dic_music_artist_female_en.h>
#include <dic_music_artist_male_en.h>
#include <algorithm>
#include <string>
#include <set>
#include <map>
#include "CJudgeMusic.h"
#include "JSONObject.h"
#include "common.h"
#include "dic_artist.h"
#include "dic_semantic.h"
#include "config.h"
#include "CSpotify.h"

using namespace std;

CJudgeMusic::CJudgeMusic()

{
	int nIndex = 0;
//	mappFnGetArtist[nIndex] = &CJudgeMusic::getArtistTaiwan;
//	mappFnGetArtist[++nIndex] = &CJudgeMusic::getArtistEnglish;
//	mappFnGetArtist[++nIndex] = &CJudgeMusic::getArtistEnglishFemale;
//	//mappFnGetArtist[++nIndex] = &CJudgeMusic::getArtistEnglishMale;

	setArtist.insert(setArtistEnglishFemale);
	setArtist.insert(setArtistEnglishMale);
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

	strArtist = getArtist(szInput);

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

//======== 評估歌手 ===========//
	strArtist = getArtist(szWord);
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

string CJudgeMusic::getArtist(const char *szWord)
{
	string strWord;
	string strValue;
	string strArtist;
	set<set<string> >::iterator it_set;
	set<string>::iterator it_set2;

	if(!strWord.empty())
	{
		transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);
		for(it_set = setArtist.begin(); setArtist.end() != it_set; ++it_set)
		{
			for(it_set2 = it_set->begin(); it_set->end() != it_set2; ++it_set2)
			{
				strValue = *it_set2;
				transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
				if(string::npos != strWord.find(strValue))
				{
					strArtist = strValue;
					_log("[CJudgeMusic] getArtist Find Artist: %s", strValue.c_str());
					break;
				}
			}
		}
	}

	return strArtist;
}

/*
 string CJudgeMusic::getArtist(const char *szWord, set<string> &setData)
 {
 string strWord;
 string strValue;
 string strArtist;
 set<string>::const_iterator it_set;

 if(!strWord.empty())
 {
 transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);
 for(it_set = setData.begin(); setData.end() != it_set; ++it_set)
 {
 strValue = *it_set;
 transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
 if(string::npos != strWord.find(strValue))
 {
 strArtist = strValue;
 _log("[CJudgeMusic] getArtist Find Artist: %s", strValue.c_str());
 break;
 }
 }
 }

 return strArtist;
 }
 */
//string CJudgeMusic::getArtistTaiwan(const char *szWord)
//{
//	return getArtist(szWord, setArtistTaiwan);
//}
//
//string CJudgeMusic::getArtistEnglish(const char *szWord)
//{
//	return getArtist(szWord, setArtistEnglish);
//}
//
//string CJudgeMusic::getArtistEnglishFemale(const char *szWord)
//{
//	return getArtist(szWord, setArtistEnglishFemale);
//}
//
//string CJudgeMusic::getArtistEnglishMale(const char *szWord)
//{
//	return "ss"; //getArtist(szWord, setArtistEnglishMale);
//}
