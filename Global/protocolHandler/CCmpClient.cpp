/*
 * CCmpClient.cpp
 *
 *  Created on: May 4, 2017
 *      Author: joe
 */

#include <netinet/in.h>
#include <string>
#include "CCmpClient.h"
#include "LogHandler.h"
#include "packet.h"
#include "event.h"
#include "CMessageHandler.h"

CCmpClient::CCmpClient() :
		confCmpClient(0)
{
	mapFunc[authentication_request] = &CCmpClient::onAuthenticationRequest;
	mapFunc[smart_building_qrcode_tokn_request] = &CCmpClient::onSmartBuildingQrCodeTokenRequest;
	mapFunc[smart_building_appversion_request] = &CCmpClient::onSmartBuildingAppVersionRequest;
	mapFunc[smart_building_getmeetingdata_request] = &CCmpClient::onSmartBuildingMeetingDataRequest;
	mapFunc[smart_building_amx_control_access_request] = &CCmpClient::onSmartBuildingAMXControlAccessRequest;

	confCmpClient = new CONF_CMP_CLIENT;
	confCmpClient->init();
}

CCmpClient::~CCmpClient()
{
	//_log("[CCmpClient] ~CCmpClient");
	stop();
	delete confCmpClient;
}

void CCmpClient::onTimer(int nId)
{

}

void CCmpClient::setUseQueueReceive(bool bEnable)
{
	confCmpClient->bUseQueueReceive = bEnable;
}

void CCmpClient::onReceive(unsigned long int nSocketFD, int nDataLen, const void* pData)
{
	CMP_HEADER cmpHeader;
	CMP_HEADER *pHeader;
	pHeader = (CMP_HEADER *) pData;
	char *pBody;
	int nHeaderSize;

	nHeaderSize = sizeof(CMP_HEADER);
	memset(&cmpHeader, 0, nHeaderSize);

	cmpHeader.command_id = ntohl(pHeader->command_id);
	cmpHeader.command_length = ntohl(pHeader->command_length);
	cmpHeader.command_status = ntohl(pHeader->command_status);
	cmpHeader.sequence_number = ntohl(pHeader->sequence_number);

	_log("[CCmpClient] onReceive %u", cmpHeader.command_id);

	if (0 >= cmpHeader.command_length || MAX_SOCKET_READ < cmpHeader.command_length)
	{
		_log("[CCmpClient] onTcpReceive receive invaild packet");
		response(nSocketFD, cmpHeader.command_id, STATUS_RINVCMDLEN, cmpHeader.sequence_number, 0);
		return;
	}

	if (nHeaderSize < cmpHeader.command_length)
	{
		pBody = (char*) ((char *) const_cast<void*>(pData) + nHeaderSize);
	}
	else
	{
		pBody = 0;
	}
	if ( generic_nack == (generic_nack & cmpHeader.command_id))
	{
		_log("[CCmpClient] onReceive Response");

		// This is response package.
		printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
				"[CCmpClient] onReceive Response ", nSocketFD);
		onResponse(nSocketFD, cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, pBody);
		return;
	}
	else
	{
		_log("[CCmpClient] onReceive Request");
		printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
				"[CCmpClient] onReceive Request ", nSocketFD);
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if (mapFunc.end() == iter)
	{
		response(nSocketFD, cmpHeader.command_id, STATUS_RINVCMDID, cmpHeader.sequence_number, 0);
		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pBody);

}

int CCmpClient::request(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData)
{
	return sendPacket(nSocket, nCommand, nStatus, nSequence, szData);
}
int CCmpClient::response(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData)
{
	return sendPacket(nSocket, nCommand | generic_nack, nStatus, nSequence, szData);
}

int CCmpClient::sendPacket(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData)
{
	int nDataLen = 0;
	int nResult = 0;
	int nBody_len = 0;
	CMP_HEADER *pHeader;
	char *pIndex;

	if (szData)
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

	if (nDataLen)
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
	printPacket(nCommand, nStatus, nSequence, nResult, "[CCmpClient] sendPacket", nSocket);

	if (nDataLen)
	{
		pIndex = buffer;
		pIndex += sizeof(CMP_HEADER);
		_log("[CCmpClient] sendPacket Body: %s", pIndex);
	}

	if (0 >= nResult)
	{
		_log("[CCmpClient] CMP response Fail socket: %d", nSocket);
	}
	else
	{
		sendMessage(getEventFilter(), EVENT_COMMAND_SOCKET_TCP_CONNECT_ALIVE, nSocket, 0, 0);
	}
	return nResult;
}
void CCmpClient::idleTimeout(bool bRun, int nIdleTime)
{
	setIdleTimeout(nIdleTime);
	runIdleTimeout(bRun);
}

int CCmpClient::onTcpReceive(unsigned long int nSocketFD)
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
	int nHeaderSize;

	nHeaderSize = sizeof(CMP_HEADER);

	pHeader = &cmpHeader;
	result = socketrecv(nSocketFD, nHeaderSize, &pHeader);

	_log("[CCmpClient] onTcpReceive result = %d", result);

	if (0 >= result)
		return 0;
	if (nHeaderSize == result)
	{
		nTotalLen = ntohl(cmpHeader.command_length);

		if (0 >= nTotalLen || MAX_SOCKET_READ < nTotalLen)
		{
			_log("[CCmpClient] onTcpReceive receive invaild packet");
			return 0;
		}

		nCommand = ntohl(cmpHeader.command_id);
		nSequence = ntohl(cmpHeader.sequence_number);
		nStatus = ntohl(cmpHeader.command_status);

		if (enquire_link_request == nCommand)
		{
			return response(nSocketFD, nCommand, STATUS_ROK, nSequence, 0);
		}

		map<int, MemFn>::iterator iter;
		iter = mapFunc.find(nCommand);

		// if a request && unkonown request
		if (mapFunc.end() == iter && (generic_nack != (generic_nack & nCommand)))
		{
			return response(nSocketFD, nCommand, STATUS_RINVCMDID, nSequence, 0);
		}

		//=================== Get CMP Body ===================//
		nBodyLen = nTotalLen - nHeaderSize;
		char buffer[nBodyLen];
		if (0 < nBodyLen)
		{
			pBody = buffer;
			memset(buffer, 0, sizeof(buffer));
			result = socketrecv(nSocketFD, nBodyLen, &pBody);
			if (result != nBodyLen)
			{
				response(nSocketFD, nCommand, STATUS_RSYSERR, nSequence, 0);
				_log("[CCmpClient] onTcpReceive System Error, Body Length: %d Receive: %d data: %s",
					nBodyLen, result, pBody);
				return 0;
			}
		}

		if (confCmpClient->bUseQueueReceive)
		{
			if (DATA_LEN < nBodyLen) // large data
			{
				if ( generic_nack == (generic_nack & nCommand))
				{
					printPacket(nCommand, nStatus, nSequence, nTotalLen, "[CCmpClient] onTcpReceive Response ",
							nSocketFD);
					onResponse(nSocketFD, nCommand, nStatus, nSequence, pBody);
				}
				else
				{
					printPacket(nCommand, nStatus, nSequence, nTotalLen, "[CCmpClient] onTcpReceive Request ",
							nSocketFD);
					if (mapFunc.end() != iter)
					{
						(this->*this->mapFunc[nCommand])(nSocketFD, nCommand, nSequence, pBody);
					}
					else
					{
						return response(nSocketFD, nCommand, STATUS_RINVCMDID, nSequence, 0);
					}
				}
			}
			else
			{
				char pBuf[DATA_LEN];
				char* pvBuf = pBuf;

				memset(pBuf, 0, sizeof(pBuf));
				memcpy(pvBuf, pHeader, nHeaderSize);
				if (nBodyLen)
				{
					pvBuf += nHeaderSize;
					memcpy(pvBuf, pBody, nBodyLen);
				}
				sendMessage(getEventFilter(), EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nSocketFD, nTotalLen,
						pBuf);
			}
		}
		else
		{
			//================== Check CMP Response ===================//
			if ( generic_nack == (generic_nack & nCommand))
			{
				printPacket(nCommand, nStatus, nSequence, nTotalLen, "[CCmpClient] onReceive Response ", nSocketFD);
				onResponse(nSocketFD, nCommand, nStatus, nSequence, pBody);
			}
			else
			{
				printPacket(nCommand, nStatus, nSequence, nTotalLen, "[CCmpClient] onReceive Request ", nSocketFD);
				(this->*this->mapFunc[nCommand])(nSocketFD, nCommand, nSequence, pBody);
			}
		}
	}
	else
	{
		response(nSocketFD, nCommand, STATUS_RINVCMDLEN, nSequence, 0);
		return 0;
	}

	return result;
}
