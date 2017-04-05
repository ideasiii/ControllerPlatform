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

#include "iCommand.h"
#include "ICallback.h"

class CServerAMX: public CSocketServer
{
public:
	static CServerAMX * getInstance();
	virtual ~CServerAMX();
	int startServer(std::string strIP, const int nPort, const int nMsqId);
	void stopServer();
	int sendCommand(std::string strCommand);
	int sendCommand(const int nSocketFD, std::string strCommand);
	void bind(const int nSocketFD);
	void unbind(const int nSocketFD);
	bool onReceive(const int nSocketFD, std::string strCommand);
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);
	void setCallback(const int nId, CBFun cbfun);

private:
	CServerAMX();
	std::map<int, int> mapClient;
	std::map<int, CBFun> mapCallback;

};
