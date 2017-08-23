/*
 * CDispatcher.h
 *
 *  Created on: 2017年3月7日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <map>
#include "CCmpServer.h"

#define ID_SERVER_SIGNIN		0
#define ID_SERVER_TRACKER		1

typedef struct _SERVER
{
	int nPort;
	std::string strName;
	std::string strIp;
} SERVER;

class CDispatcher: public CCmpServer
{
public:
	static CDispatcher* getInstance();
	virtual ~CDispatcher();
	CDispatcher &addServer(int nId, const char *szName, const char *szIp, int nPort);
	void createResp();

protected:
	int onInitial(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onDie(int nSocket, int nCommand, int nSequence, const void *szBody);

private:
	explicit CDispatcher();
	std::map<int, SERVER> mapServer;
	std::string mstrResp;

};
