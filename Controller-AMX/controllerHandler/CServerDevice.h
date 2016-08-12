/*
 * CServerDevice.h
 *
 *  Created on: 2016年8月9日
 *      Author: root
 */

#pragma once

#include <string>

class CSocketServer;

using namespace std;

class CServerDevice
{
public:
	static CServerDevice * getInstance();
	virtual ~CServerDevice();
	int startServer(const int nPort, const int nMsqId);
	void stopServer();
	int sendCommand(const int nSocketFD, string strCommand);

private:
	CServerDevice();
	CSocketServer * socketServer;
};
