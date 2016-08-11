/*
 * CServerAMX.h
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#pragma once

#include <string>

class CSocketServer;

using namespace std;

class CServerAMX
{
public:
	static CServerAMX * getInstance();
	virtual ~CServerAMX();
	int startServer(const int nPort, const int nMsqId);
	void stopServer();
	int sendCommand(string strCommand);
	void bind(const int nSocketFD);
	void unbind(const int nSocketFD);

private:
	CServerAMX();
	CSocketServer * socketServer;
	int mnSocketAMX;
};
