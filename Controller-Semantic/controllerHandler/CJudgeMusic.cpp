/*
 * CJudgeMusic.cpp
 *
 *  Created on: 2017年5月11日
 *      Author: root
 */

#include <algorithm>
#include <string>
#include <map>
#include <set>
#include "CJudgeMusic.h"
#include "JSONObject.h"
#include "common.h"
#include "dictionary.h"
#include "config.h"
#include "CSpotify.h"
#include "CFileHandler.h"
#include "utility.h"

using namespace std;

CJudgeMusic::CJudgeMusic() :
		spotify(0)
{
	loadArtistDictionary();
	spotify = new CSpotify;
}

CJudgeMusic::~CJudgeMusic()
{
	delete spotify;
}

string CJudgeMusic::toString()
{
	return "CJudgeMusic";
}

int CJudgeMusic::word(const char *szInput, JSONObject* jsonResp, map<string, string> &mapMatch)
{
	int nResult;
	int nIndex;
	string strArtist;
	string strAlbum;
	string strTrack;
	string strTrackUri;
	string strCover;
	string strWord;
	JSONObject jsonSpotify;
	set<string>::const_iterator iter;
	map<string, ALBUM> mapAlbums;
	map<string, ALBUM>::const_iterator it;
	map<int, TRACK> mapSong;

	strWord = trim(szInput);
	transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);

	strArtist = trim(getArtist(szInput));

	if(!strArtist.empty())
	{
		nResult = spotify->getAlbum(strArtist.c_str(), mapAlbums, "TW");
		if(ERROR_STATUS_NO_TOKEN == nResult)
		{
			spotify->authorization(SPOTIFY_CLIENT);
			nResult = spotify->getAlbum(strArtist.c_str(), mapAlbums, "TW");
		}
		if(0 < nResult)
		{
			for(it = mapAlbums.begin(); mapAlbums.end() != it; ++it)
			{
				_log("[CJudgeMusic] word get %s - %s -- %s", strArtist.c_str(), it->first.c_str(),
						it->second.id.c_str());
				strCover = it->second.strCover;

				mapSong.clear();

				nResult = spotify->getTrack(it->second.id.c_str(), mapSong, "TW");
				if(ERROR_STATUS_NO_TOKEN == nResult)
				{
					spotify->authorization(SPOTIFY_CLIENT);
					nResult = spotify->getTrack(it->second.id.c_str(), mapSong, "TW");
				}

				if(0 < nResult)
				{
					for(map<int, TRACK>::const_iterator cit = mapSong.begin(); mapSong.end() != cit; ++cit)
					{
						_log("			 %s - %s", it->first.c_str(), cit->second.name.c_str());
						strAlbum = it->first;
						strTrack = cit->second.name;
						strTrackUri = cit->second.uri;

						for(iter = setArtistMark.begin(); setArtistMark.end() != iter; ++iter)
						{
							nIndex = strTrack.find(*iter);
							if((int) string::npos != nIndex)
								strTrack = trim(strTrack.substr(0, nIndex));
						}

						transform(strTrack.begin(), strTrack.end(), strTrack.begin(), ::tolower);
						if(string::npos != strWord.find(strTrack))
						{
							if(!strTrackUri.empty())
							{
								jsonSpotify.put("source", 2);
								jsonSpotify.put("album", strAlbum);
								jsonSpotify.put("artist", strArtist);
								jsonSpotify.put("song", strTrack);
								jsonSpotify.put("id", strTrackUri);
								jsonSpotify.put("cover", strCover);
								jsonResp->put("type", TYPE_RESP_MUSIC);
								jsonResp->put("music", jsonSpotify);
							}
							return TRUE;
						}
					}
				}

			}
		}
	}

	if(!strTrackUri.empty())
	{
		jsonSpotify.put("source", 2);
		jsonSpotify.put("album", strAlbum);
		jsonSpotify.put("artist", strArtist);
		jsonSpotify.put("song", strTrack);
		jsonSpotify.put("id", strTrackUri);
		jsonSpotify.put("cover", strCover);
		jsonResp->put("type", TYPE_RESP_MUSIC);
		jsonResp->put("music", jsonSpotify);
	}
	else
	{
		jsonResp->put("type", TYPE_RESP_TTS);
		if(strArtist.empty())
			strWord = "無此歌手的樂曲";
		else
			strWord = format("歌手%s的歌曲因版權問題暫時無法播放", strArtist.c_str());
		jsonResp->put("tts", strWord);
	}

	return TRUE;
}

int CJudgeMusic::evaluate(const char *szWord, map<string, string> &mapMatch)
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
	string strArtist;
	set<string>::iterator it_set;
	map<string, string>::const_iterator it_map;

	strWord = trim(szWord);

	if(!strWord.empty())
	{
		transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);
		for(it_set = setArtist.begin(); setArtist.end() != it_set; ++it_set)
		{
			strArtist = trim(*it_set);
			transform(strArtist.begin(), strArtist.end(), strArtist.begin(), ::tolower);
			if(!strArtist.empty() && 0 < strArtist.length() && string::npos != strWord.find(strArtist))
			{
				_log("[CJudgeMusic] getArtist Find Artist: %s", strArtist.c_str());
				return strArtist;
			}
		}

		for(it_map = mapArtistMatch.begin(); mapArtistMatch.end() != it_map; ++it_map)
		{
			strArtist = trim(it_map->first);
			transform(strArtist.begin(), strArtist.end(), strArtist.begin(), ::tolower);
			if(!strArtist.empty() && 0 < strArtist.length() && string::npos != strWord.find(strArtist))
			{
				strArtist = trim(it_map->second);
				_log("[CJudgeMusic] getArtist Find Artist: %s", strArtist.c_str());
				return strArtist;
			}
		}
	}

	strArtist.clear();
	return strArtist;
}

void CJudgeMusic::loadArtistDictionary()
{
	string strPath;
	CFileHandler fh;
	set<string> setData;
	set<string>::const_iterator iter;
	int nIndex;
	int nCount;
	string strWord;
	string strMatch;

	fh.readPath("dictionary", setData);

	for(iter = setData.begin(); setData.end() != iter; ++iter)
	{
		nIndex = iter->find("artist");
		if((int) string::npos != nIndex && 0 == nIndex)
		{
			strPath = format("dictionary/%s", iter->c_str());
			nCount = fh.readAllLine(strPath.c_str(), setArtist);
			_log("[CJudgeMusic] loadArtistDictionary read file: %s count: %d", strPath.c_str(), nCount);
		}
	}
	_log("[CJudgeMusic] loadArtistDictionary Total Artist Count: %d", setArtist.size());

	setData.clear();
	fh.readAllLine("dictionary/match_tw_en_artist.txt", setData);
	for(iter = setData.begin(); setData.end() != iter; ++iter)
	{
		if(!iter->empty())
		{
			nIndex = iter->find(",");
			strWord = iter->substr(0, nIndex);
			strMatch = iter->substr(nIndex + 1);
			mapArtistMatch[strWord] = strMatch;
		}
	}
	_log("[CJudgeMusic] loadArtistDictionary Total Artist Match Count: %d", mapArtistMatch.size());
}
