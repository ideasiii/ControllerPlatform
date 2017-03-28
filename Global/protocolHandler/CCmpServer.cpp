/*
 * CCmpServer.cpp
 *
 *  Created on: 2017年3月16日
 *      Author: Jugo
 */

#include <netinet/in.h>
#include <string>
#include "CCmpServer.h"
#include "LogHandler.h"
#include "packet.h"
#include "event.h"
#include "CMessageHandler.h"

using namespace std;

CCmpServer::CCmpServer()
{
	mapFunc[initial_request] = &CCmpServer::onInitial;
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
				"[CCmpServer] onReceive Response ", nSocketFD);
		return;
	}
	else
	{
		printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
				"[CCmpServer] onReceive Request ", nSocketFD);
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
	return sendPacket(nSocket, nCommand, nStatus, nSequence, szData);
}
int CCmpServer::response(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData)
{
	return sendPacket(nSocket, nCommand | generic_nack, nStatus, nSequence, szData);
}

int CCmpServer::sendPacket(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData)
{
	updateClientAlive(nSocket);

	int nDataLen = 0;
	int nResult = 0;
	int nBody_len = 0;
	CMP_HEADER *pHeader;
	char *pIndex;

<<<<<<< HEAD
	memset(&packet, 0, sizeof(CMP_PACKET));

	packet.cmpHeader.command_id = htonl(nCommand);
	packet.cmpHeader.command_status = htonl(nStatus);
	packet.cmpHeader.sequence_number = htonl(nSequence);

	if(szData)
	{
		packet.cmpBodyUnlimit.cmpdata = new char[strlen(szData) + 1];
		pIndex = packet.cmpBodyUnlimit.cmpdata;
		memcpy(pIndex, szData, strlen(szData));
		pIndex += strlen(szData);
		nBody_len += strlen(szData);
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		//delete packet.cmpBodyUnlimit.cmpdata;
	}

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);
	nResult = socketSend(nSocket, &packet, nTotal_len);

	printPacket(nCommand, nStatus, nSequence, nResult, "[CCmpServer] request", nSocket);

	if(0 >= nResult)
	{
		_log("[CCmpServer] CMP request Fail socket: %d", nSocket);
=======
	if (szData)
	{
		nDataLen = strlen(szData) + 1;
>>>>>>> c39beb7d798bfccc3ddb01425b4ce4db807c8a42
	}

	char buffer[sizeof(CMP_HEADER) + nDataLen];
	memset(buffer, 0, sizeof(buffer));

	pIndex = buffer;
	pHeader = (CMP_HEADER *) buffer;

	pHeader->command_id = htonl(nCommand);
	pHeader->command_status = htonl(nStatus);
	pHeader->sequence_number = htonl(nSequence);

<<<<<<< HEAD
	if(szData)
	{
//		packet.cmpBodyUnlimit.cmpdata = new char[strlen(szData) + 1];
//		memset(packet.cmpBodyUnlimit.cmpdata, 0, strlen(szData));
		pIndex = packet.cmpBody.cmpdata;
		memcpy(packet.cmpBody.cmpdata, szData, strlen(szData));
=======
	if (nDataLen)
	{
		pIndex += sizeof(CMP_HEADER);
		memcpy(pIndex, szData, strlen(szData));
>>>>>>> c39beb7d798bfccc3ddb01425b4ce4db807c8a42
		pIndex += strlen(szData);
		nBody_len += strlen(szData);
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		_log("[xxxxxxxx] %s", packet.cmpBody.cmpdata);
	}

	pHeader->command_length = htonl(sizeof(buffer));
	nResult = socketSend(nSocket, buffer, sizeof(buffer));
	printPacket(nCommand, nStatus, nSequence, nResult, "[CCmpServer] sendPacket", nSocket);

	if(0 >= nResult)
	{
		_log("[CCmpServer] CMP response Fail socket: %d", nSocket);
	}
//	if(szData)
//		delete packet.cmpBodyUnlimit.cmpdata;
	return nResult;
}
void CCmpServer::idleTimeout(bool bRun, int nIdleTime)
{
	setIdleTimeout(nIdleTime);
	runIdleTimeout(bRun);
}

int CCmpServer::onTcpReceive(unsigned long int nSocketFD)
{
//======= Receive CMP Header ==========//
	int result;
	CMP_HEADER cmpHeader;
	void *pHeader;
	void *pBody;
	int nTotalLen = 0;
	int nBodyLen = 0;
	int nCommand = generic_nack;
	int nSequence = 0;

	pHeader = &cmpHeader;
	result = socketrecv(nSocketFD, sizeof(CMP_HEADER), &pHeader);
	if (sizeof(CMP_HEADER) == result)
	{
		nTotalLen = ntohl(cmpHeader.command_length);
		nCommand = ntohl(cmpHeader.command_id);
		nSequence = ntohl(cmpHeader.sequence_number);

		if ( enquire_link_request == nCommand)
		{
			return response(nSocketFD, nCommand, STATUS_ROK, nSequence, 0);
		}

		nBodyLen = nTotalLen - sizeof(CMP_HEADER);
		char buffer[nBodyLen];

		if (0 < nBodyLen)
		{
			pBody = buffer;
			memset(buffer, 0, sizeof(buffer));
			result = socketrecv(nSocketFD, nBodyLen, &pBody);
			if (result != nBodyLen)
			{
				response(nSocketFD, nCommand, STATUS_RSYSERR, nSequence, 0);
				return 0;
			}
		}

		if (DATA_LEN < nBodyLen) // big data
		{
			map<int, MemFn>::iterator iter;
			iter = mapFunc.find(nCommand);
			if (mapFunc.end() == iter)
			{
				result = response(nSocketFD, nCommand, STATUS_RINVCMDID, nSequence, 0);
			}
			else
				(this->*this->mapFunc[nCommand])(nSocketFD, nCommand, nSequence, pBody);
		}
		else
		{
			char pBuf[DATA_LEN];
			char* pvBuf = pBuf;

			memset(pBuf, 0, sizeof(pBuf));
			memcpy(pvBuf, pHeader, sizeof(CMP_HEADER));
			if (nBodyLen)
			{
				pvBuf += sizeof(CMP_HEADER);
				memcpy(pvBuf, pBody, nBodyLen);
			}
			sendMessage(getEventId(), EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nSocketFD, nTotalLen, pBuf);
		}
	}
	else
	{
		response(nSocketFD, nCommand, STATUS_RINVCMDLEN, nSequence, 0);
		return 0;
	}

	return result;
}

