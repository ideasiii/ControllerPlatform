/*
 * CContentHandler.cpp
 *
 *  Created on: 2017年8月8日
 *      Author: Jugo
 */

#include <string>
#include "CContentHandler.h"
#include "utility.h"
#include "config.h"
#include "common.h"

using namespace std;

extern set<string> setArtistMark;

CContentHandler::CContentHandler() :
		spotify(0)
{
	spotify = new CSpotify;
}

CContentHandler::~CContentHandler()
{
	delete spotify;
}

int CContentHandler::spotifyTrack(const char *szInput, const char *szArtist, TRACK &track)
{
	int nResult;
	int nIndex;
	int nRand;
	int nCount;
	string strArtist;
	string strAlbum;
	string strTrack;
	string strTrackUri;
	string strCover;
	string strWord;

	set<string>::const_iterator iter;
	map<string, ALBUM> mapAlbums;
	map<string, ALBUM>::const_iterator it;
	map<int, TRACK> mapSong;
	map<int, TRACK> mapAllTrack;

	strWord = trim(szInput);
	transform(strWord.begin(), strWord.end(), strWord.begin(), ::tolower);
	strArtist = trim(szArtist);
	transform(strArtist.begin(), strArtist.end(), strArtist.begin(), ::tolower);

	track.clear();
	if(!strArtist.empty())
	{
		nCount = 0;
		nResult = spotify->getAlbum(strArtist.c_str(), mapAlbums, "TW");
		if(ERROR_STATUS_NO_TOKEN == nResult)
		{
			spotify->authorization(SPOTIFY_CLIENT);
			nResult = spotify->getAlbum(strArtist.c_str(), mapAlbums, "TW");
		}
		if(0 < nResult)
		{
			// 取得專輯
			for(it = mapAlbums.begin(); mapAlbums.end() != it; ++it)
			{
				_log("[CContentHandler] spotifyTrack get %s - %s -- %s", strArtist.c_str(), it->first.c_str(),
						it->second.id.c_str());
				strCover = it->second.strCover;

				mapSong.clear();

				// 抓取專輯每首歌
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
						strAlbum = it->first;
						strTrack = cit->second.name;
						strTrackUri = cit->second.uri;

						for(iter = setArtistMark.begin(); setArtistMark.end() != iter; ++iter)
						{
							nIndex = strTrack.find(*iter);
							if((int) string::npos != nIndex)
								strTrack = trim(strTrack.substr(0, nIndex));
						}

						nIndex = strTrack.find("(", 1);
						if((int) string::npos != nIndex)
						{
							strTrack = trim(strTrack.substr(0, nIndex));
						}

						_log("			 %s - %s", strAlbum.c_str(), strTrack.c_str());

						track.clear();
						track.artist = strArtist;
						track.album = strAlbum;
						track.name = strTrack;
						track.id = strTrackUri;
						track.strCover = strCover;
						mapAllTrack[nCount++] = track;

						// 搜尋要聽的曲名
						transform(strTrack.begin(), strTrack.end(), strTrack.begin(), ::tolower);
						if(string::npos != strWord.find(strTrack))
						{
							mapAlbums.clear();
							mapSong.clear();
							mapAllTrack.clear();
							return TRUE;
						}
					}
				}
			}
		}
	}

	if(mapAllTrack.size())
	{
		nRand = getRand(0, mapAllTrack.size() - 1);
		strTrackUri = mapAllTrack[nRand].id;
		if(!strTrackUri.empty())
		{
			track.clear();
			track.artist = mapAllTrack[nRand].artist;
			track.album = mapAllTrack[nRand].album;
			track.name = mapAllTrack[nRand].name;
			track.id = mapAllTrack[nRand].id;
			track.strCover = mapAllTrack[nRand].strCover;
			return TRUE;
		}
	}

	mapAlbums.clear();
	mapSong.clear();
	mapAllTrack.clear();

	return FALSE;
}

void CContentHandler::getWeather(const char *szLocation, WEATHER &weather)
{
	CWeather wt;

	if(szLocation)
	{
		wt.getWeather(szLocation, weather);
	}
}

void CContentHandler::getNews(NEWS_DATE &newsDate)
{
	CNewsHandler news;

	if(!newsDate.strDate.empty())
	{
		if(newsDate.strDate.compare(currentDate()))
		{
			news.getNewsToday(newsDate);
		}
	}
	else
		news.getNewsToday(newsDate);
}

