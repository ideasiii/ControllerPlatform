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
#include "ICallback.h"

using namespace std;

class CServerAMX: public CSocketServer
{
public:
	static CServerAMX * getInstance();
	virtual ~CServerAMX();
	int startServer(string strIP, const int nPort, const int nMsqId);
	void stopServer();
	int sendCommand(string strCommand);
	int sendCommand(const int nSocketFD, string strCommand);
	void bind(const int nSocketFD);
	void unbind(const int nSocketFD);
	bool onReceive(const int nSocketFD, string strCommand);
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);
	void setCallback(const int nId, CBFun cbfun);

private:
	CServerAMX();
	map<int, int> mapClient;
	map<int, CBFun> mapCallback;

};
