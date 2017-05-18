/*
 * CSpotify.cpp
 *
 *  Created on: 2017年4月24日
 *      Author: root
 */

#include <string>
#include "CSpotify.h"
#include "CHttpsClient.h"
#include "utility.h"
#include "JSONObject.h"
#include "JSONArray.h"

#define QUERY_ARTIST		"https://api.spotify.com/v1/search?q=%s&type=artist"
#define QUERY_ALBUM			"https://api.spotify.com/v1/search?q=%s&type=album"
#define QUERY_TRACK			"https://api.spotify.com/v1/search?q=%s&type=track"
#define QUERY_TRACK_ALBUM	"https://api.spotify.com/v1/albums/%s/tracks?limit=50"

using namespace std;

CSpotify::CSpotify()
{

}

CSpotify::~CSpotify()
{

}

int CSpotify::getAlbum(const char *szArtist, std::map<std::string, std::string> &mapAlbums)
{

	if(0 == szArtist)
		return 0;

	CHttpsClient *httpsClient = new CHttpsClient;
	string strData;
	string strURL = format(QUERY_ALBUM, urlEncode(szArtist).c_str());
	httpsClient->GET(strURL.c_str(), strData);
	delete httpsClient;

	if(!strData.empty())
	{
		string strAlbum;
		string strAlbumId;
		JSONObject *jroot = new JSONObject(strData);
		JSONObject *jAlbums = new JSONObject(jroot->getJsonObject("albums"));
		JSONArray *jItems = new JSONArray(jAlbums->getJsonArray("items"));
		for(int i = 0; i < jItems->size(); ++i)
		{
			JSONObject *jAlbum = new JSONObject(jItems->getJsonObject(i));
			if(0 == jAlbum->getString("type").compare("album"))
			{
				strAlbum = jAlbum->getString("name");
				strAlbumId = jAlbum->getString("id");
				if(!strAlbum.empty() && !strAlbumId.empty())
					mapAlbums[strAlbum] = strAlbumId;
			}
			jAlbum->release();
			delete jAlbum;
		}
		jItems->release();
		jAlbums->release();
		jroot->release();
		delete jItems;
		delete jAlbums;
		delete jroot;
	}

	return mapAlbums.size();
}

/*
 * "href": "https://api.spotify.com/v1/tracks/6Y2XecuoGvqs5iHhkxQ1OU",
 "id": "6Y2XecuoGvqs5iHhkxQ1OU",
 "name": "千言萬語",
 "popularity": 33,
 "preview_url": "https://p.scdn.co/mp3-preview/972d630c82c37aca213def7115946e23756e9e43?cid=null",
 "track_number": 1,
 "type": "track",
 "uri": "spotify:track:6Y2XecuoGvqs5iHhkxQ1OU"*/
int CSpotify::getTrack(const char *szAlbumId, std::map<int, TRACK> &mapSong)
{
	if(0 == szAlbumId)
		return 0;

	CHttpsClient *httpsClient = new CHttpsClient;
	string strData;
	string strURL = format(QUERY_TRACK_ALBUM, szAlbumId);
	httpsClient->GET(strURL.c_str(), strData);
	delete httpsClient;

	if(!strData.empty())
	{
		string strSong;
		JSONObject *jroot = new JSONObject(strData);
		JSONArray *jItems = new JSONArray(jroot->getJsonArray("items"));
		for(int i = 0; i < jItems->size(); ++i)
		{
			JSONObject *jTrack = new JSONObject(jItems->getJsonObject(i));
			if(0 == jTrack->getString("type").compare("track"))
			{
				TRACK track;
				track.href = jTrack->getString("href");
				track.id = jTrack->getString("id");
				track.name = jTrack->getString("name");
				track.popularity = jTrack->getInt("popularity");
				track.preview_url = jTrack->getString("preview_url");
				track.track_number = jTrack->getInt("track_number");
				track.uri = jTrack->getString("uri");
				mapSong[jTrack->getInt("track_number")] = track;
			}
			jTrack->release();
			delete jTrack;
		}
		jItems->release();

		jroot->release();
		delete jItems;
		delete jroot;
	}

	return mapSong.size();
}

