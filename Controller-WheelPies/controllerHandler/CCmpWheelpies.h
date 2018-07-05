/*
 * CCmpWheelpies.h
 *
 *  Created on: 2018年7月4日
 *      Author: root
 */

#pragma once

#include "CCmpServer.h"

class CCmpWheelpies: public CCmpServer
{
public:
	explicit CCmpWheelpies(CObject *object);
	virtual ~CCmpWheelpies();

protected:
	int onWheelpies(int nSocket, int nCommand, int nSequence, const void *szBody);

private:
	CObject *mpController;
};

