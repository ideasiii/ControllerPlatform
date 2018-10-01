/*
 * CDomainServer.h
 *
 *  Created on: 2018年10月1日
 *      Author: Jugo
 */

#pragma once

#include "CSocket.h"
#include "CObject.h"

class CDomainServer: public CSocket, public CObject
{
public:
	int start(const char* szSocketFile, int nMsqKey = -1);
	virtual std::string taskName();
	void runSocketAccept();
	void runMessageReceive();
	void runTcpReceive();
private:
	int DOMAIN_SERVER_MSQ_EVENT_FILTER;
	std::string strTaskName;
	int mnMsqKey; // Message queue key and filter ID.
};
