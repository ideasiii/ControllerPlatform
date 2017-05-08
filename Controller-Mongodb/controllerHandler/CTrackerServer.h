/*
 * CTrackerServer.h
 *
 *  Created on: 2017年3月31日
 *      Author: root
 */

#pragma once

#include "CCmpServer.h"

class CObject;

class CTrackerServer: public CCmpServer
{
public:
	explicit CTrackerServer(CObject *object);
	virtual ~CTrackerServer();

protected:
	int onAccesslog(int nSocket, int nCommand, int nSequence, const void *szBody);
	CObject *mpController;
};
