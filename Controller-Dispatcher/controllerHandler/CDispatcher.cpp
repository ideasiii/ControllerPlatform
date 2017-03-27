/*
 * CDispatcher.cpp
 *
 *  Created on: 2017年3月7日
 *      Author: root
 */

#include "CDispatcher.h"
#include "common.h"
#include "LogHandler.h"
#include "packet.h"
#include <string>

using namespace std;

static string RESP_DISPATCH =
		"{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"175.98.119.121\",\"port\": 2306	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"175.98.119.121\",\"port\": 2307}]}";

static CDispatcher * dispatcher = 0;

CDispatcher* CDispatcher::getInstance()
{
	if (0 == dispatcher)
	{
		dispatcher = new CDispatcher();
	}
	return dispatcher;
}

CDispatcher::CDispatcher()
{

}

CDispatcher::~CDispatcher()
{

}

int CDispatcher::onInitial(int nSocket, int nCommand, int nSequence, const void *szData)
{
	return response(nSocket, nCommand, STATUS_ROK, nSequence, RESP_DISPATCH.c_str());
}

int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

