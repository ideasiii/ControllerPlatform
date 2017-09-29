/*
 * CContentHandler.h
 *
 *  Created on: 2017年8月8日
 *      Author: Jugo
 */

#pragma once

#include "CSpotify.h"
#include "CWeather.h"
#include "CNewsHandler.h"

class CContentHandler
{
public:
	CContentHandler();
	virtual ~CContentHandler();
	int spotifyTrack(const char *szInput, const char *szArtist, TRACK &track);
	void getWeather(const char *szLocation, WEATHER &weather);
	void getNews(NEWS_DATE &newsDate);

private:
	CSpotify *spotify;
};
