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

__attribute__ ((unused)) static int sendPacket(CSocket *socket, const int nSocket, const int nCommandId,
		const int nStatus, const int nSequence, const char * szData, bool isBigData = false)
{
	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	char *pIndex;

	memset(&packet, 0, sizeof(CMP_PACKET));

	packet.cmpHeader.command_id = htonl(nCommandId);
	packet.cmpHeader.command_status = htonl(nStatus);
	packet.cmpHeader.sequence_number = htonl(nSequence);

	if (isBigData == true)
	{
		char *reallyBigBuffer = new char[sizeof(CMP_HEADER) + strlen(szData) + 2];

		/* codes below are copy paste */
		nTotal_len = sizeof(CMP_HEADER);

		if (0 != szData)
		{
			memcpy(reallyBigBuffer + sizeof(CMP_HEADER), szData, strlen(szData));
			*(reallyBigBuffer + sizeof(CMP_HEADER) + strlen(szData)) = '\0';
			nTotal_len += strlen(szData) + 1;
		}

		packet.cmpHeader.command_length = htonl(nTotal_len);
		memcpy(reallyBigBuffer, (void*) &packet.cmpHeader, sizeof(CMP_HEADER));

		nRet = socket->socketSend(nSocket, reallyBigBuffer, nTotal_len);
		printPacket(nCommandId, STATUS_ROK, nSequence, nRet, "[Packet] Send", nSocket);

		string strLog;
		if (0 >= nRet)
		{
			_log("[CCmpServer] CMP Send Fail socket: %d", nSocket);
		}

		delete reallyBigBuffer;
		return nRet;
	}
	else
	{
		pIndex = packet.cmpBody.cmpdata;

		/* codes below are copy paste */
		if (0 != szData)
		{
			memcpy(pIndex, szData, strlen(szData));
			pIndex += strlen(szData);
			nBody_len += strlen(szData);
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
		}

		nTotal_len = sizeof(CMP_HEADER) + nBody_len;
		packet.cmpHeader.command_length = htonl(nTotal_len);
		nRet = socket->socketSend(nSocket, &packet, nTotal_len);
		printPacket(nCommandId, STATUS_ROK, nSequence, nRet, "[Packet] Send", nSocket);

		string strLog;
		if (0 >= nRet)
		{
			_log("[CCmpServer] CMP Send Fail socket: %d", nSocket);
		}

		return nRet;
	}

}

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

void CCmpServer::onReceive(unsigned long int nId, int nDataLen, const void* pData)
{
	_log("[CCmpServer] onReceive, Socket FD: %lu", nId);
	CMP_HEADER cmpHeader;
	char *pPacket;

	CMP_HEADER *pHeader;
	pHeader = (CMP_HEADER *) pData;

	pPacket = (char*) const_cast<void*>(pData);
	memset(&cmpHeader, 0, sizeof(CMP_HEADER));

	cmpHeader.command_id = ntohl(pHeader->command_id);
	cmpHeader.command_length = ntohl(pHeader->command_length);
	cmpHeader.command_status = ntohl(pHeader->command_status);
	cmpHeader.sequence_number = ntohl(pHeader->sequence_number);

	if ( generic_nack == (generic_nack & cmpHeader.command_id))
	{
		printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
				"[CCmpServer] CMP Response ", nId);
		return;
	}
	else
	{
		printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
				"[CCmpServer] CMP Request ", nId);
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if (0x000000FF < cmpHeader.command_id || 0x00000000 >= cmpHeader.command_id || mapFunc.end() == iter)
	{
		sendPacket(dynamic_cast<CSocket*>(this), nId, generic_nack | cmpHeader.command_id,
		STATUS_RINVCMDID, cmpHeader.sequence_number, 0);
		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nId, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);

}

