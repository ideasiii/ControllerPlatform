/*
 * CSpotify.h
 *
 *  Created on: 2017年4月24日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <set>
#include <map>

#define ERROR_STATUS_ERROR		-1
#define ERROR_STATUS_SUCCESS	0
#define ERROR_STATUS_NO_TOKEN	401

typedef struct _ALBUM
{
	std::string name;
	std::string id;
	std::string strCover;
	void clear()
	{
		name.clear();
		id.clear();
		strCover.clear();
	}
} ALBUM;

typedef struct _TRACK
{
	int track_number;
	int popularity;
	std::string artist;
	std::string album;
	std::string href;
	std::string id;
	std::string name;
	std::string preview_url;
	std::string uri;
	std::string strCover;
	void clear()
	{
		track_number = -1;
		popularity = -1;
		href.clear();
		id.clear();
		name.clear();
		preview_url.clear();
		uri.clear();
		strCover.clear();
		artist.clear();
		album.clear();
	}
} TRACK;

class CSpotify
{

public:
	CSpotify();
	virtual ~CSpotify();
	int getAlbum(const char *szArtist, std::map<std::string, ALBUM> &mapAlbums, const char *szAvailableMarket = 0);
	int getTrack(const char *szAlbumId, std::map<int, TRACK> &mapSong, const char *szAvailableMarket = 0);
	void authorization(const char* szClient);

private:
	int checkError(const char *szJSONResp);

private:
	std::string mstrAccessToken;

};
