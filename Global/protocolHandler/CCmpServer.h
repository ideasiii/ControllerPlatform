/*
 * CCmpServer.h
 *
 *  Created on: 2017年3月16日
 *      Author: Jugo
 */

#pragma once

#include "CATcpServer.h"

class CCmpServer: public CATcpServer
{
public:
	CCmpServer();
	virtual ~CCmpServer();

protected:
	void onTimer(int nId);
	void onReceive(unsigned long int nId, int nDataLen, const void* pData);

};
