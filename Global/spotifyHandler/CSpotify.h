/*
 * CSpotify.h
 *
 *  Created on: 2017年4月24日
 *      Author: root
 */

#pragma once

#include <set>
#include <map>

typedef struct _TRACK
{
	int track_number;
	int popularity;
	std::string href;
	std::string id;
	std::string name;
	std::string preview_url;
	std::string uri;
} TRACK;

class CSpotify
{

public:
	CSpotify();
	virtual ~CSpotify();
	int getAlbum(const char *szArtist, std::map<std::string, std::string> &mapAlbums,
			const char *szAvailableMarket = 0);
	int getTrack(const char *szAlbumId, std::map<int, TRACK> &mapSong, const char *szAvailableMarket = 0);
};
