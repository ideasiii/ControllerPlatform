/*
 * CCmpServer.cpp
 *
 *  Created on: 2017年3月16日
 *      Author: Jugo
 */

#include <netinet/in.h>
#include "CCmpServer.h"
#include "LogHandler.h"
#include "packet.h"

using namespace std;

CCmpServer::CCmpServer()
{
	mapFunc[sign_up_request] = &CCmpServer::onSignin;
	mapFunc[access_log_request] = &CCmpServer::onAccesslog;
}

CCmpServer::~CCmpServer()
{
	stop();
}

void CCmpServer::onTimer(int nId)
{

}

void CCmpServer::onReceive(unsigned long int nSocketFD, int nDataLen, const void* pData)
{
	CMP_HEADER cmpHeader;
	CMP_HEADER *pHeader;
	pHeader = (CMP_HEADER *) pData;

	memset(&cmpHeader, 0, sizeof(CMP_HEADER));

	cmpHeader.command_id = ntohl(pHeader->command_id);
	cmpHeader.command_length = ntohl(pHeader->command_length);
	cmpHeader.command_status = ntohl(pHeader->command_status);
	cmpHeader.sequence_number = ntohl(pHeader->sequence_number);

	if( generic_nack == (generic_nack & cmpHeader.command_id))
	{
		printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
				"[CCmpServer] CMP Response ", nSocketFD);
		return;
	}
	else
	{
		printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
				"[CCmpServer] CMP Request ", nSocketFD);
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if(0x000000FF < cmpHeader.command_id || 0x00000000 >= cmpHeader.command_id || mapFunc.end() == iter)
	{
		response(nSocketFD, cmpHeader.command_id, STATUS_RINVCMDID, cmpHeader.sequence_number, 0);
		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pData);

}

int CCmpServer::request(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData)
{
	int nResult = 0;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	char *pIndex;

	memset(&packet, 0, sizeof(CMP_PACKET));

	packet.cmpHeader.command_id = htonl(nCommand);
	packet.cmpHeader.command_status = htonl(nStatus);
	packet.cmpHeader.sequence_number = htonl(nSequence);

	if(szData)
	{
		packet.cmpBodyUnlimit.cmpdata = new char(strlen(szData) + 1);
		pIndex = packet.cmpBodyUnlimit.cmpdata;
		memcpy(pIndex, szData, strlen(szData));
		pIndex += strlen(szData);
		nBody_len += strlen(szData);
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		delete packet.cmpBodyUnlimit.cmpdata;
	}

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);
	nResult = socketSend(nSocket, &packet, nTotal_len);

	printPacket(nCommand, nStatus, nSequence, nResult, "[CCmpServer] request", nSocket);

	if(0 >= nResult)
	{
		_log("[CCmpServer] CMP request Fail socket: %d", nSocket);
	}

	return nResult;
}
int CCmpServer::response(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData)
{
	int nResult = 0;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	char *pIndex;

	memset(&packet, 0, sizeof(CMP_PACKET));

	packet.cmpHeader.command_id = htonl(nCommand | generic_nack);
	packet.cmpHeader.command_status = htonl(nStatus);
	packet.cmpHeader.sequence_number = htonl(nSequence);

	if(szData)
	{
		packet.cmpBodyUnlimit.cmpdata = new char(strlen(szData) + 1);
		pIndex = packet.cmpBodyUnlimit.cmpdata;
		memcpy(pIndex, szData, strlen(szData));
		pIndex += strlen(szData);
		nBody_len += strlen(szData);
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		delete packet.cmpBodyUnlimit.cmpdata;
	}

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);
	nResult = socketSend(nSocket, &packet, nTotal_len);
	printPacket(nCommand | generic_nack, nStatus, nSequence, nResult, "[CCmpServer] response", nSocket);

	if(0 >= nResult)
	{
		_log("[CCmpServer] CMP response Fail socket: %d", nSocket);
	}

	return nResult;
}

void CCmpServer::idleTimeout(bool bRun, int nIdleTime)
{
	setIdleTimeout(nIdleTime);
	runIdleTimeout(bRun);
}

