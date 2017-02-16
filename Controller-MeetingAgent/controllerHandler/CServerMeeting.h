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

class CServerMeeting: public CSocketServer
{
public:
	static CServerMeeting * getInstance();
	virtual ~CServerMeeting();
	int startServer(string strIP, const int nPort, const int nMsqId);
	void stopServer();
	bool onReceive(const int nSocketFD, const void *pData);



private:
	CServerMeeting();


};
