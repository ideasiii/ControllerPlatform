/*
 * CDomainServer.cpp
 *
 *  Created on: 2018年10月1日
 *      Author: Jugo
 */

#include <thread>
#include "CDomainServer.h"
#include "CMessageHandler.h"
#include "LogHandler.h"
#include "event.h"
#include "utility.h"
#include "common.h"

using namespace std;

int CDomainServer::start(const char* szSocketFile, int nMsqKey)
{
	static int nEventFilter = EVENT_FILTER_DOMAIN_SERVER;
	int nMsgId = -1;
	int nSocketFD;
	//int mnExtMsqKey = FALSE;

	DOMAIN_SERVER_MSQ_EVENT_FILTER = ++nEventFilter;
	strTaskName = taskName();

	if(-1 != nMsqKey)
	{
		mnMsqKey = nMsqKey;
		//mnExtMsqKey = TRUE;
	}
	else
		mnMsqKey = clock();

	if(-1 == mnMsqKey)
		mnMsqKey = 20150727;

	nMsgId = initMessage(mnMsqKey, strTaskName.c_str());

	if(-1 == nMsgId)
	{
		_log("[CDomainServer] start Init Message Queue Fail");
		return -1;
	}

	setDomainSocketPath(szSocketFile);
	nSocketFD = createSocket(AF_UNIX, SOCK_DGRAM);

	if(-1 != nSocketFD)
	{
		if(-1 != socketBind())
		{
			if(-1 == socketListen(BACKLOG))
			{
				perror("socket listen");
				socketClose();
				return -1;
			}

			thread([=]
			{	runMessageReceive( );}).detach();
			thread([=]
			{	runSocketAccept( );}).detach();
			//createThread(threadCATcpServerMessageReceive, this, "CATcpServer Message Receive");
			//createThread(threadTcpAccept, this, "CATcpServer Socket Accept Thread");
			_log("[CDomainServer] %s Create Server Success Domain Socket File: %d Socket FD: %lu", strTaskName.c_str(),
					szSocketFile, nSocketFD);
		}
		else
		{
			socketClose();
		}
	}
	else
		_log("[CDomainServer] %s Create Server Fail", strTaskName.c_str());

	return nSocketFD;
}

string CDomainServer::taskName()
{
	return "CDomainServer";
}

void CDomainServer::runSocketAccept()
{

}
void CDomainServer::runMessageReceive()
{

}

void CDomainServer::runTcpReceive()
{

}

