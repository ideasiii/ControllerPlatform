/*
 * CTrackerServer.cpp
 *
 *  Created on: 2017年3月31日
 *      Author: root
 */

#include "CTrackerServer.h"
#include "common.h"
#include "LogHandler.h"
#include "packet.h"
#include <string>

using namespace std;

static CTrackerServer * trackerServer = 0;

CTrackerServer::CTrackerServer()
{

}

CTrackerServer::~CTrackerServer()
{

}

CTrackerServer* CTrackerServer::getInstance()
{
	if(0 == trackerServer)
	{
		trackerServer = new CTrackerServer();
	}
	return trackerServer;
}

int CTrackerServer::onAccessLog(int nSocket, int nCommand, int nSequence, const void *szData)
{
	return response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
}

int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

