/*
 * CDispatcher.h
 *
 *  Created on: 2017年3月7日
 *      Author: root
 */

#pragma once

#include <map>
#include "CSocketServer.h"

class CDispatcher: public CSocketServer
{
public:
	static CDispatcher* getInstance();
	virtual ~CDispatcher();
	int startServer(const char *szIP, const int nPort, const int nMsqId);
	void stopServer();
	void onReceiveMessage(unsigned long int nId, int nDataLen, const void* pData);
	void setClient(unsigned long int nId, bool bAdd);
	void checkClient();

private:
	explicit CDispatcher();
	std::map<unsigned long int, long> listClient;
};
