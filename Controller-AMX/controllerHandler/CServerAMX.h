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
	int sendCommand(const int nSocketFD, string strCommand);
	void bind(const int nSocketFD);
	void unbind(const int nSocketFD);
	bool onReceive(const int nSocketFD, string strCommand);

private:
	CServerAMX();
	CSocketServer * socketServer;
	int mnSocketAMX;
};
