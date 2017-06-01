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
#include "LogHandler.h"

#define QUERY_ARTIST		"https://api.spotify.com/v1/search?q=%s&type=artist"
#define QUERY_ALBUM			"https://api.spotify.com/v1/search?q=%s&type=album"
#define QUERY_TRACK			"https://api.spotify.com/v1/search?q=%s&type=track"
#define QUERY_TRACK_ALBUM	"https://api.spotify.com/v1/albums/%s/tracks?limit=50"
#define QUERY_TOKEN			"https://accounts.spotify.com/api/token"

using namespace std;

CSpotify::CSpotify()
{

}

CSpotify::~CSpotify()
{

}

int CSpotify::getAlbum(const char *szArtist, std::map<std::string, std::string> &mapAlbums,
		const char *szAvailableMarket)
{
	string strToken;
	set<string> setHead;
	CHttpsClient *httpsClient;
	string strData;
	string strURL;

	if(!szArtist)
		return 0;
	if(mstrAccessToken.empty())
		return -1;

	httpsClient = new CHttpsClient;
	strURL = format(QUERY_ALBUM, urlEncode(szArtist).c_str());

	setHead.insert(mstrAccessToken);
	httpsClient->GET(strURL.c_str(), strData, setHead);
	delete httpsClient;

	if(!strData.empty())
	{
		string strAlbum;
		string strAlbumId;
		JSONObject *jroot = new JSONObject(strData);
		JSONObject *jAlbums = new JSONObject(jroot->getJsonObject("albums"));
		if(!jAlbums->isValid())
		{
			_log("[CSpotify] getAlbum Error: %s", strData.c_str());
			jAlbums->release();
			jroot->release();
			delete jAlbums;
			delete jroot;
			return -1;
		}
		JSONArray *jItems = new JSONArray(jAlbums->getJsonArray("items"));
		for(int i = 0; i < jItems->size(); ++i)
		{
			JSONObject *jAlbum = new JSONObject(jItems->getJsonObject(i));
			JSONArray *jarrAM = new JSONArray(jAlbum->getJsonArray("available_markets"));
			if(szAvailableMarket)
			{
				for(int i = 0; i < jarrAM->size(); ++i)
				{
					if(!jarrAM->getString(i).compare(szAvailableMarket))
					{
						if(0 == jAlbum->getString("type").compare("album"))
						{
							strAlbum = jAlbum->getString("name");
							strAlbumId = jAlbum->getString("id");
							if(!strAlbum.empty() && !strAlbumId.empty())
								mapAlbums[strAlbum] = strAlbumId;
						}
					}
				}
			}
			else
			{
				if(0 == jAlbum->getString("type").compare("album"))
				{
					strAlbum = jAlbum->getString("name");
					strAlbumId = jAlbum->getString("id");
					if(!strAlbum.empty() && !strAlbumId.empty())
						mapAlbums[strAlbum] = strAlbumId;
				}
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

int CSpotify::getTrack(const char *szAlbumId, std::map<int, TRACK> &mapSong, const char *szAvailableMarket)
{
	CHttpsClient *httpsClient;
	string strData;
	string strURL;
	set<string> setHead;

	if(0 == szAlbumId)
		return 0;
	if(mstrAccessToken.empty())
		return -1;

	httpsClient = new CHttpsClient;
	strURL = format(QUERY_TRACK_ALBUM, szAlbumId);

	setHead.insert(mstrAccessToken);
	httpsClient->GET(strURL.c_str(), strData, setHead);
	delete httpsClient;

	if(!strData.empty())
	{
		string strSong;
		JSONObject *jroot = new JSONObject(strData);
		JSONArray *jItems = new JSONArray(jroot->getJsonArray("items"));
		if(!jItems->isValid())
		{
			_log("[CSpotify] getTrack Error: %s", strData.c_str());
			jroot->release();
			delete jroot;
			return -1;
		}
		for(int i = 0; i < jItems->size(); ++i)
		{
			JSONObject *jTrack = new JSONObject(jItems->getJsonObject(i));
			if(0 == jTrack->getString("type").compare("track"))
			{
				TRACK track;
				if(szAvailableMarket)
				{
					JSONArray *jarrAM = new JSONArray(jTrack->getJsonArray("available_markets"));
					for(int i = 0; i < jarrAM->size(); ++i)
					{
						if(!jarrAM->getString(i).compare(szAvailableMarket))
						{
							track.href = jTrack->getString("href");
							track.id = jTrack->getString("id");
							track.name = jTrack->getString("name");
							track.popularity = jTrack->getInt("popularity");
							track.preview_url = jTrack->getString("preview_url");
							track.track_number = jTrack->getInt("track_number");
							track.uri = jTrack->getString("uri");
							mapSong[jTrack->getInt("track_number")] = track;
						}
					}
					jarrAM->release();
					delete jarrAM;
				}
				else
				{
					track.href = jTrack->getString("href");
					track.id = jTrack->getString("id");
					track.name = jTrack->getString("name");
					track.popularity = jTrack->getInt("popularity");
					track.preview_url = jTrack->getString("preview_url");
					track.track_number = jTrack->getInt("track_number");
					track.uri = jTrack->getString("uri");
					mapSong[jTrack->getInt("track_number")] = track;
				}
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

void CSpotify::authorization(const char* szClient)
{
	CHttpsClient *httpsClient;
	string strData;
	string strClient;
	string strToken;
	string strError;
	set<string> setHead;
	set<string> setParameter;

	if(!szClient)
		return;

	httpsClient = new CHttpsClient;

	strClient = format("Authorization: Basic %s", szClient);
	setHead.insert(strClient.c_str());
	setParameter.insert("grant_type=client_credentials");

	httpsClient->POST(QUERY_TOKEN, strData, setHead, setParameter);
	_log("[CSpotify] authorization token: %s", strData.c_str());

	JSONObject *jroot = new JSONObject(strData);
	if(jroot->isValid())
	{
		strToken = jroot->getString("access_token");
		if(strToken.empty())
		{
			strError = jroot->getString("error");
			if(!strError.empty())
			{
				_log("[CSpotify] authorization Error: %s", strError.c_str());
			}
		}
		else
		{
			mstrAccessToken = format("Authorization: Bearer %s", strToken.c_str());
			_log("[CSpotify] authorization Access Token: %s", mstrAccessToken.c_str());
		}
	}
	jroot->release();
	delete jroot;

//============= Success ================//
// {"access_token":"BQC4QZN_jZwsv_zZYzu06wzNhev2BUTGlpjzMNYYMujqyyRRhqbVaCm7uQ1ExsKLS1LWuW4vIZaXGFEoGU1sjg","token_type":"Bearer","expires_in":3600}
//============= Error ==================//
// {"error":"invalid_client","error_description":"Invalid client"}

}

