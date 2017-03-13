/*
 * CDispatcher.cpp
 *
 *  Created on: 2017年3月7日
 *      Author: root
 */

#include "CDispatcher.h"
#include "common.h"
#include "LogHandler.h"
#include "event.h"
#include "IReceiver.h"
#include "packet.h"
#include "CCmpHandler.h"
#include <time.h>

#define TIMER_CHECK_DISPATCH_CLIENT_ALIVE 777
#define RESP_DISPATCH "{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"175.98.119.121\",\"port\": 2306	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"175.98.119.121\",\"port\": 2307}]}"

static CDispatcher * dispatcher = 0;

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

CDispatcher* CDispatcher::getInstance()
{
	if(0 == dispatcher)
	{
		dispatcher = new CDispatcher();
	}
	return dispatcher;
}

CDispatcher::CDispatcher() :
		cmpParser(CCmpHandler::getInstance())
{
	mapFunc[initial_request] = &CDispatcher::cmpInitial;
}

CDispatcher::~CDispatcher()
{

}

int CDispatcher::startServer(const char *szIP, const int nPort, const int nMsqId)
{
	if(0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if(0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_DISPATCHER_RECEIVER);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DISPATCHER);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DISPATCHER);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	setPacketConf(PK_CMP, PK_MSQ);

	if(FAIL == start(AF_INET, szIP, nPort))
	{
		_log("[CDispatcher] Socket Create Fail");
		return FALSE;
	}

	setTimer(TIMER_CHECK_DISPATCH_CLIENT_ALIVE, 5, 10);

	return TRUE;
}

void CDispatcher::stopServer()
{
	this->stop();
}

void CDispatcher::onReceiveMessage(unsigned long int nSocketFD, int nDataLen, const void* pData)
{
	int nRet = -1;
	int nPacketLen = 0;
	CMP_HEADER cmpHeader;
	char *pPacket;

	if(0 >= pData)
	{
		sendPacket(dynamic_cast<CSocket*>(dispatcher), nSocketFD, generic_nack, STATUS_RINVCMP, getSequence(), 0);
	}

	pPacket = (char*) const_cast<void*>(pData);
	memset(&cmpHeader, 0, sizeof(CMP_HEADER));

	_TRY
		cmpHeader.command_id = cmpParser->getCommand(pPacket);
		cmpHeader.command_length = cmpParser->getLength(pPacket);
		cmpHeader.command_status = cmpParser->getStatus(pPacket);
		cmpHeader.sequence_number = cmpParser->getSequence(pPacket);
	_CATCH

	printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
			"[Server Center] Recv ", nSocketFD);

	if(cmpParser->isAckPacket(cmpHeader.command_id))
	{
		return;
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if(MAX_COMMAND < cmpHeader.command_id || 0x00000000 >= cmpHeader.command_id || mapFunc.end() == iter)
	{
		sendPacket(dynamic_cast<CSocket*>(dispatcher), nSocketFD, generic_nack | cmpHeader.command_id,
		STATUS_RINVCMDID, cmpHeader.sequence_number, 0);
		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);
}

void CDispatcher::setClient(unsigned long int nId, bool bAdd)
{
	if(bAdd)
	{
		listClient[nId] = (long) clock();
		_DBG("[CDispatcher] add Socket Client: %d", (int )nId);
	}
	else
	{
		listClient.erase(nId);
		_DBG("[CDispatcher] Erase Socket Client: %d", (int )nId);
	}
}

void CDispatcher::checkClient()
{
	for(map<unsigned long int, long>::iterator it = listClient.begin(); it != listClient.end(); ++it)
	{
		_DBG("[]");
		if(10 <= (((double) (clock() - (*it).second)) / 100))
		{
			closeClient((*it).first);
			setClient((*it).first, false);
		}
	}
}

int CDispatcher::cmpInitial(int nSocket, int nCommand, int nSequence, const void *pData)
{
	return sendPacket(dynamic_cast<CSocket*>(dispatcher), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence,
	RESP_DISPATCH);
}

void CDispatcher::onTimer(int nId)
{
	checkClient();
}

