/*
 * CTrackerServer.h
 *
 *  Created on: 2017年3月31日
 *      Author: root
 */

#pragma once

#include "CCmpServer.h"



class CTrackerServer: public CCmpServer
{
public:
	static CTrackerServer* getInstance();
	virtual ~CTrackerServer();

protected:
	int onAccessLog(int nSocket, int nCommand, int nSequence, const void *szData);

private:
	explicit CTrackerServer();

};
