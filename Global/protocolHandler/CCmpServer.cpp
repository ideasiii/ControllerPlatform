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

CCmpServer::CCmpServer() :
		confCmpServer(0)
{
	mapFunc[initial_request] = &CCmpServer::onInitial;
	mapFunc[sign_up_request] = &CCmpServer::onSignin;
	mapFunc[access_log_request] = &CCmpServer::onAccesslog;
	mapFunc[semantic_word_request] = &CCmpServer::onSemanticWord;
	mapFunc[fcm_id_register_request] = &CCmpServer::onFCMIdRegister;
	mapFunc[facebook_token_client_request] = &CCmpServer::onFBToken;
	mapFunc[smart_building_qrcode_tokn_request] = &CCmpServer::onQRCodeToken;
	mapFunc[smart_building_appversion_request] = &CCmpServer::onAPPVersion;
	mapFunc[smart_building_getmeetingdata_request] = &CCmpServer::onGetMeetingData;
	mapFunc[smart_building_amx_control_access_request] = &CCmpServer::onAMXControlAccess;
	mapFunc[smart_building_wireless_power_charge_request] = &CCmpServer::onWirelessPowerCharge;

	confCmpServer = new CONF_CMP_SERVER;
	confCmpServer->init();
}

CCmpServer::~CCmpServer()
{
	stop();
	delete confCmpServer;
}

void CCmpServer::onTimer(int nId)
{

}

void CCmpServer::setUseQueueReceive(bool bEnable)
{
	confCmpServer->bUseQueueReceive = bEnable;
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

	if(0 >= cmpHeader.command_length || MAX_SOCKET_READ < cmpHeader.command_length)
	{
		_log("[CCmpServer] onTcpReceive receive invaild packet");
		response(nSocketFD, cmpHeader.command_id, STATUS_RINVCMDLEN, cmpHeader.sequence_number, 0);
		return;
	}

	if( generic_nack == (generic_nack & cmpHeader.command_id))
	{
		// This is response package.
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

	if(mapFunc.end() == iter)
	{
		response(nSocketFD, cmpHeader.command_id, STATUS_RINVCMDID, cmpHeader.sequence_number, 0);
		return;
	}

	char *pBody = (char*) ((char *) const_cast<void*>(pData) + sizeof(CMP_HEADER));
	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pBody);

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
	int nDataLen = 0;
	int nResult = 0;
	int nBody_len = 0;
	CMP_HEADER *pHeader;
	char *pIndex;

	if(szData)
	{
		nDataLen = strlen(szData) + 1;
	}

	char buffer[sizeof(CMP_HEADER) + nDataLen];
	memset(buffer, 0, sizeof(buffer));

	pIndex = buffer;
	pHeader = (CMP_HEADER *) buffer;

	pHeader->command_id = htonl(nCommand);
	pHeader->command_status = htonl(nStatus);
	pHeader->sequence_number = htonl(nSequence);

	if(nDataLen)
	{
		pIndex += sizeof(CMP_HEADER);
		memcpy(pIndex, szData, strlen(szData));
		pIndex += strlen(szData);
		nBody_len += strlen(szData);
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
	}

	pHeader->command_length = htonl(sizeof(buffer));
	nResult = socketSend(nSocket, buffer, sizeof(buffer));
	printPacket(nCommand, nStatus, nSequence, nResult, "[CCmpServer] sendPacket", nSocket);

	if(nDataLen)
	{
		pIndex = buffer;
		pIndex += sizeof(CMP_HEADER);
		_log("[CCmpServer] sendPacket Body: %s", pIndex);
	}

	if(0 >= nResult)
	{
		_log("[CCmpServer] CMP response Fail socket: %d", nSocket);
	}
	else
		sendMessage(EVENT_FILTER_SOCKET_SERVER, EVENT_COMMAND_SOCKET_TCP_CONNECT_ALIVE, nSocket, 0, 0);

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
	void *pBody = 0;
	int nTotalLen = 0;
	int nBodyLen = 0;
	int nCommand = generic_nack;
	int nSequence = 0;
	int nStatus;

	pHeader = &cmpHeader;
	result = socketrecv(nSocketFD, sizeof(CMP_HEADER), &pHeader);
	if(0 >= result)
		return 0;
	if(sizeof(CMP_HEADER) == result)
	{
		nTotalLen = ntohl(cmpHeader.command_length);

		if(0 >= nTotalLen || MAX_SOCKET_READ < nTotalLen)
		{
			_log("[CCmpServer] onTcpReceive receive invaild packet");
			return 0;
		}

		nCommand = ntohl(cmpHeader.command_id);
		nSequence = ntohl(cmpHeader.sequence_number);
		nStatus = ntohl(cmpHeader.command_status);

		if( enquire_link_request == nCommand)
		{
			return response(nSocketFD, nCommand, STATUS_ROK, nSequence, 0);
		}

		nBodyLen = nTotalLen - sizeof(CMP_HEADER);

		char buffer[nBodyLen];

		if(0 < nBodyLen)
		{
			pBody = buffer;
			memset(buffer, 0, sizeof(buffer));
			result = socketrecv(nSocketFD, nBodyLen, &pBody);
			if(result != nBodyLen)
			{
				response(nSocketFD, nCommand, STATUS_RSYSERR, nSequence, 0);
				_log("[CCmpServer] onTcpReceive System Error, Body Length: %d Receive: %d data: %s", nBodyLen, result,
						pBody);
				return 0;
			}
		}

		printPacket(nCommand, nStatus, nSequence, nTotalLen, "[CCmpServer] onTcpReceive ", nSocketFD);

		if(confCmpServer->bUseQueueReceive)
		{
			if(DATA_LEN < nBodyLen) // large data
			{
				map<int, MemFn>::iterator iter;
				iter = mapFunc.find(nCommand);
				if(mapFunc.end() == iter)
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
				if(nBodyLen)
				{
					pvBuf += sizeof(CMP_HEADER);
					memcpy(pvBuf, pBody, nBodyLen);
				}
				sendMessage(EVENT_FILTER_SOCKET_SERVER, EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nSocketFD, nTotalLen,
						pBuf);
			}
		}
		else
		{
			(this->*this->mapFunc[nCommand])(nSocketFD, nCommand, nSequence, pBody);
		}
	}
	else
	{
		response(nSocketFD, nCommand, STATUS_RINVCMDLEN, nSequence, 0);
		return 0;
	}

	return result;
}
