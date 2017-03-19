/*
 * CCmpSignin.h
 *
 *  Created on: 2017年3月16日
 *      Author: root
 */

#pragma once

#include "CCmpServer.h"

class CCmpSignin: public CCmpServer
{
public:
	CCmpSignin();
	virtual ~CCmpSignin();

protected:
	int onSignin(int nSocket, int nCommand, int nSequence, const void *pData);
};
