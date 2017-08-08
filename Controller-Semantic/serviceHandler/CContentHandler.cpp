/*
 * CContentHandler.cpp
 *
 *  Created on: 2017年8月8日
 *      Author: root
 */

#include "CContentHandler.h"
#include "CSpotify.h"

CContentHandler::CContentHandler() :
		spotify(0)
{
	spotify = new CSpotify;
}

CContentHandler::~CContentHandler()
{
	delete spotify;
}

