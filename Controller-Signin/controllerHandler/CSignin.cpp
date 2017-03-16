/*
 * CSignin.cpp
 *
 *  Created on: 2017年3月14日
 *      Author: Jugo
 */

#include "CSignin.h"
#include "common.h"
#include "LogHandler.h"
#include "event.h"
#include "IReceiver.h"
#include "packet.h"
#include "CCmpHandler.h"
#include <time.h>

static CSignin *signin = 0;

#define TIMER_CHECK_SIGNIN_CLIENT_ALIVE			20170314

/**
 * Define Socket Client ReceiveFunction
 */
int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

/**
 *  Define Socket Server Receive Function
 */
int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

CSignin::CSignin() :
		cmpParser(CCmpHandler::getInstance())
{
	mapFunc[sign_up_request] = &CSignin::cmpSignin;
}

CSignin::~CSignin()
{

}

CSignin* CSignin::getInstance()
{
	if(!signin)
	{
		signin = new CSignin();
	}
	return signin;
}

int CSignin::startServer(const char *szIP, const int nPort, const int nMsqId)
{
	if(0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if(0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_SIGNIN_RECEIVER);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_SIGNIN);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_SIGNIN);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	setPacketConf(PK_CMP, PK_MSQ);

	if(FAIL == start(AF_INET, szIP, nPort))
	{
		_log("[CDispatcher] Socket Create Fail");
		return FALSE;
	}

	setTimer(TIMER_CHECK_SIGNIN_CLIENT_ALIVE, 5, 10);

	return TRUE;
}

void CSignin::stopServer()
{

}

void CSignin::onReceiveMessage(unsigned long int nSocketFD, int nDataLen, const void* pData)
{

}

void CSignin::setClient(unsigned long int nId, bool bAdd)
{
	if(bAdd)
	{
		listClient[nId] = (long) clock();
		_DBG("[CSignin] add Socket Client: %d", (int )nId);
	}
	else
	{
		listClient.erase(nId);
		_DBG("[CSignin] Erase Socket Client: %d", (int )nId);
	}
}

void CSignin::checkClient()
{
	for(map<unsigned long int, long>::iterator it = listClient.begin(); it != listClient.end(); ++it)
	{
		if(10 <= (((double) (clock() - (*it).second)) / 100))
		{
			closeClient((*it).first);
			setClient((*it).first, false);
		}
	}
}

void CSignin::onTimer(int nId)
{
	checkClient();
}

int CSignin::cmpSignin(int nSocket, int nCommand, int nSequence, const void *pData)
{
	return TRUE;
}

