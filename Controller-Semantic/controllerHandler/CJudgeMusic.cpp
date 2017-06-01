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
#include "config.h"
#include "CSpotify.h"
#include "CFileHandler.h"

using namespace std;

CJudgeMusic::CJudgeMusic()
{
	loadArtistDictionary();
}

CJudgeMusic::~CJudgeMusic()
{

}

int CJudgeMusic::word(const char *szInput, JSONObject* jsonResp)
{
	int nResult;
	string strArtist;
	string strAlbum;
	string strTrack;
	string strTrackUri;

	strArtist = getArtist(szInput);

	if(!strArtist.empty())
	{
		CSpotify spotify;
		map<string, string> mapAlbums;
		map<string, string>::const_iterator it;
		nResult = spotify.getAlbum(strArtist.c_str(), mapAlbums, "TW");
		if(-1 == nResult)
		{
			spotify.authorization(SPOTIFY_CLIENT);
		}
		if(0 < spotify.getAlbum(strArtist.c_str(), mapAlbums, "TW"))
		{
			for(it = mapAlbums.begin(); mapAlbums.end() != it; ++it)
			{
				_log("[CJudgeMusic] word get %s - %s -- %s", strArtist.c_str(), it->first.c_str(), it->second.c_str());
				map<int, TRACK> mapSong;
				if(-1 == spotify.getTrack(it->second.c_str(), mapSong, "TW"))
				{
					break;
				}
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
	strArtist = getArtist(strWord.c_str());
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

	strWord = szWord;

	if(!strWord.empty())
	{
		for(it_set = setArtist.begin(); setArtist.end() != it_set; ++it_set)
		{
			for(it_set2 = it_set->begin(); it_set->end() != it_set2; ++it_set2)
			{
				strValue = *it_set2;
				transform(strValue.begin(), strValue.end(), strValue.begin(), ::tolower);
				//_log("[CJudgeMusic] getArtist word: %s artist: %s", strWord.c_str(), strValue.c_str());
				if(!strValue.empty() && 0 < strValue.length() && string::npos != strWord.find(strValue))
				{
					strArtist = strValue;
					_log("[CJudgeMusic] getArtist Find Artist: %s", strValue.c_str());
					return strArtist;
				}
			}
		}
	}

	return strArtist;
}

void CJudgeMusic::loadArtistDictionary()
{
	CFileHandler fh;

	for(map<string, set<string> >::iterator it = mapArtistDic.begin(); mapArtistDic.end() != it; ++it)
	{
		fh.readAllLine(it->first.c_str(), it->second);
		setArtist.insert(it->second);
		_log("[CJudgeMusic] loadArtistDictionary From: %s Count: %d", it->first.c_str(), it->second.size());
	}
}
