/*
 * CContentHandler.h
 *
 *  Created on: 2017年8月8日
 *      Author: Jugo
 */

#pragma once

class CSpotify;

class CContentHandler
{
public:
	CContentHandler();
	virtual ~CContentHandler();

private:
	CSpotify *spotify;
};
