/*
 * CServerDevice.h
 *
 *  Created on: 2016年8月9日
 *      Author: root
 */

#pragma once

#include <string>

#include "CSocketServer.h"

using namespace std;

class CServerDevice: public CSocketServer
{
public:
	static CServerDevice * getInstance();
	virtual ~CServerDevice();
	int startServer(const int nPort, const int nMsqId);
	void stopServer();

private:
	CServerDevice();
};
