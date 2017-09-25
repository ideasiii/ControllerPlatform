/*
 * CContentHandler.h
 *
 *  Created on: 2017年8月8日
 *      Author: Jugo
 */

#pragma once

#include "CSpotify.h"
#include "CWeather.h"

class CContentHandler
{
public:
	CContentHandler();
	virtual ~CContentHandler();
	int spotifyTrack(const char *szInput, const char *szArtist, TRACK &track);
	void getWeather(const char *szLocation, WEATHER &weather);

private:
	CSpotify *spotify;
};
