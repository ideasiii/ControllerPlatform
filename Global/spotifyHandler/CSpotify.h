/*
 * CSpotify.h
 *
 *  Created on: 2017年4月24日
 *      Author: root
 */

#pragma once

#include <set>
#include <map>

class CSpotify
{
public:
	CSpotify();
	virtual ~CSpotify();
	int getAlbum(const char *szArtist, std::map<std::string, std::string> &mapAlbums);
	int getSong(const char *szAlbumId, std::set<std::string> &setSong);
};
