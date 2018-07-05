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
#include "utility.h"

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
	mapFunc[enquire_link_request] = &CCmpServer::onEnquireLink;
	mapFunc[bind_request] = &CCmpServer::onBind;
	mapFunc[unbind_request] = &CCmpServer::onUnbind;
	mapFunc[amx_control_request] = &CCmpServer::onAmxControl;
	mapFunc[amx_status_request] = &CCmpServer::onAmxStatus;
	mapFunc[controller_die_request] = &CCmpServer::onDie;
	mapFunc[update_request] = &CCmpServer::onUpdate;
	mapFunc[wheelpies_request] = &CCmpServer::onWheelpies;

	confCmpServer = new CONF_CMP_SERVER;
	confCmpServer->init();
	strTaskName = taskName();
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
	char *pBody;
	int nHeaderSize;

	nHeaderSize = sizeof(CMP_HEADER);
	memset(&cmpHeader, 0, nHeaderSize);

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

	if(nHeaderSize < cmpHeader.command_length)
	{
		pBody = (char*) ((char *) const_cast<void*>(pData) + nHeaderSize);
	}
	else
		pBody = 0;

	if( generic_nack == (generic_nack & cmpHeader.command_id))
	{
		// This is response package.
		printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
				"[CCmpServer] onReceive Response ", nSocketFD);
		onResponse(nSocketFD, cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, pBody);
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

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pBody);

}

int CCmpServer::request(const int nSocketFD, int nCommand, int nStatus, int nSequence, const char *szData)
{
	return sendPacket(nSocketFD, nCommand, nStatus, nSequence, szData);
}
int CCmpServer::response(const int nSocketFD, int nCommand, int nStatus, int nSequence, const char *szData)
{
	return sendPacket(nSocketFD, nCommand | generic_nack, nStatus, nSequence, szData);
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
	if(0 >= nResult)
	{
		_log("[CCmpServer] sendPacket Fail socket: %d", nSocket);
		closeClient(nSocket);
	}
	else
		sendMessage(getEventFilter(), EVENT_COMMAND_SOCKET_TCP_CONNECT_ALIVE, nSocket, 0, 0);
	printPacket(nCommand, nStatus, nSequence, nResult, "[CCmpServer] sendPacket", nSocket);

	if(nDataLen)
	{
		pIndex = buffer;
		pIndex += sizeof(CMP_HEADER);
		_log("[CCmpServer] sendPacket Body: %s", pIndex);
	}

	return nResult;
}
void CCmpServer::idleTimeout(bool bRun, int nIdleTime)
{
	setIdleTimeout(nIdleTime);
	runIdleTimeout(bRun);
}

int CCmpServer::onTcpReceive(unsigned long int nSocketFD)
{
	strTaskName = taskName();
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
	_TRACE("[CCmpServer] onTcpReceive socketrecv result: %d", result);

	if(0 >= result)
	{
		_log("[CCmpServer] onTcpReceive socketrecv fail, result: %d", result);
		return 0;
	}
	if(nHeaderSize == result)
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
			printPacket(nCommand, nStatus, nSequence, nTotalLen, "[CCmpServer] onTcpReceive Request", nSocketFD);
			return response(nSocketFD, nCommand, STATUS_ROK, nSequence, 0);
		}

		map<int, MemFn>::iterator iter;
		iter = mapFunc.find(nCommand);

		if(mapFunc.end() == iter && (generic_nack != (generic_nack & nCommand)))
		{
			return response(nSocketFD, nCommand, STATUS_RINVCMDID, nSequence, 0);
		}

		//=================== Get CMP Body ===================//
		nBodyLen = nTotalLen - nHeaderSize;
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

		if(confCmpServer->bUseQueueReceive)
		{
			if(DATA_LEN < nBodyLen) // large data
			{
				if( generic_nack == (generic_nack & nCommand))
				{
					printPacket(nCommand, nStatus, nSequence, nTotalLen, "[CCmpServer] onReceive Response ", nSocketFD);
					onResponse(nSocketFD, nCommand, nStatus, nSequence, pBody);
				}
				else
				{
					printPacket(nCommand, nStatus, nSequence, nTotalLen, "[CCmpServer] onReceive Request ", nSocketFD);
					(this->*this->mapFunc[nCommand])(nSocketFD, nCommand, nSequence, pBody);
				}
			}
			else
			{
				char pBuf[DATA_LEN];
				char* pvBuf = pBuf;

				memset(pBuf, 0, sizeof(pBuf));
				memcpy(pvBuf, pHeader, nHeaderSize);
				if(nBodyLen)
				{
					pvBuf += nHeaderSize;
					memcpy(pvBuf, pBody, nBodyLen);
				}
				sendMessage(getEventFilter(), EVENT_COMMAND_SOCKET_SERVER_RECEIVE, nSocketFD, nTotalLen, pBuf);
			}
		}
		else
		{
			//================== Check CMP Response ===================//
			if( generic_nack == (generic_nack & nCommand))
			{
				printPacket(nCommand, nStatus, nSequence, nTotalLen,
						format("[CCmpServer] %s onReceive Response ", strTaskName.c_str()).c_str(), nSocketFD);
				onResponse(nSocketFD, nCommand, nStatus, nSequence, pBody);
			}
			else
			{
				printPacket(nCommand, nStatus, nSequence, nTotalLen,
						format("[CCmpServer] %s onReceive request ", strTaskName.c_str()).c_str(), nSocketFD);
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

string CCmpServer::taskName()
{
	return "CCmpServer";
}

void CCmpServer::onClientConnect(unsigned long int nSocketFD)
{
}

void CCmpServer::onClientDisconnect(unsigned long int nSocketFD)
{
}

int CCmpServer::onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody)
{
	return 0;
}
