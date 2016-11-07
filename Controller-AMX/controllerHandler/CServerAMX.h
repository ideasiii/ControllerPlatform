/*
 * CServerAMX.h
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#pragma once

#include "CSocketServer.h"
#include <string>
#include <map>

using namespace std;

class CServerAMX: public CSocketServer
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
	void addAMXClient(const int nSocketFD);
	void deleteAMXClient(const int nSocketFD);

private:
	CServerAMX();
	map<int, int> mapClient;

};
