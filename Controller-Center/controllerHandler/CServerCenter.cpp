/*
 * CServerCenter.cpp
 *
 *  Created on: 2016-12-05
 *      Author: Jugo
 */

#include "IReceiver.h"
#include "event.h"
#include "packet.h"
#include "common.h"
#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "JSONObject.h"
#include "CServerCenter.h"
#include "ICallback.h"

static CServerCenter * serverCenter = 0;

CServerCenter::CServerCenter() :
		CSocketServer(), cmpParser(CCmpHandler::getInstance())
{
	mapFunc[bind_request] = &CServerCenter::cmpBind;
	mapFunc[unbind_request] = &CServerCenter::cmpUnbind;
	mapFunc[initial_request] = &CServerCenter::cmpInitial;
}

CServerCenter::~CServerCenter()
{
	stop();
	mapClient.clear();
}

CServerCenter * CServerCenter::getInstance()
{
	if (0 == serverCenter)
	{
		serverCenter = new CServerCenter();
	}
	return serverCenter;
}

int CServerCenter::startServer(string strIP, const int nPort, const int nMsqId)
{
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if (0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_CENTER_RECEIVE);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_CENTER);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_CENTER);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	setPacketConf(PK_CMP, PK_MSQ);

	const char* cszAddr = NULL;
	if (!strIP.empty())
		cszAddr = strIP.c_str();

	if (FAIL == start(AF_INET, cszAddr, nPort))
	{
		_log("[Server Center] Socket Create Fail");
		return FALSE;
	}

	return TRUE;
}

void CServerCenter::stopServer()
{
	stop();
}

void CServerCenter::onReceive(const int nSocketFD, const void *pData)
{
	int nRet = -1;
	int nPacketLen = 0;
	CMP_HEADER cmpHeader;
	char *pPacket;

	pPacket = (char*) const_cast<void*>(pData);
	memset(&cmpHeader, 0, sizeof(CMP_HEADER));

	cmpHeader.command_id = cmpParser->getCommand(pPacket);
	cmpHeader.command_length = cmpParser->getLength(pPacket);
	cmpHeader.command_status = cmpParser->getStatus(pPacket);
	cmpHeader.sequence_number = cmpParser->getSequence(pPacket);

	printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
			"[Server Center] Recv ", nSocketFD);

	if (cmpParser->isAckPacket(cmpHeader.command_id))
	{
		return;
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if (0x000000FF < cmpHeader.command_id || 0x00000000 >= cmpHeader.command_id || mapFunc.end() == iter)
	{
		sendCommand(dynamic_cast<CSocket*>(serverCenter), nSocketFD, generic_nack | cmpHeader.command_id,
		STATUS_RINVCMDID, cmpHeader.sequence_number);
		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);

}

int CServerCenter::cmpBind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	mapClient[nSocket] = nSocket;
	_log("[Server Center] Socket Client FD:%d Binded", nSocket);
	sendCommand(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence);
	return TRUE;
}

int CServerCenter::cmpUnbind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	if (mapClient.end() != mapClient.find(nSocket))
	{
		mapClient.erase(nSocket);
		_log("[Server Center] Socket Client FD:%d Unbinded", nSocket);
	}
	sendCommand(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence);
	return TRUE;
}

void CServerCenter::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

void CServerCenter::addClient(const int nSocketFD)
{
	_log("[Server Center] Socket Client FD:%d Connected", nSocketFD);
}

void CServerCenter::deleteClient(const int nSocketFD)
{
	if (mapClient.end() != mapClient.find(nSocketFD))
	{
		mapClient.erase(nSocketFD);
		_log("[Server Center] Socket Client FD:%d Unbinded", nSocketFD);
	}
	_log("[Server Center] Socket Client FD:%d Disconnected", nSocketFD);
}

int CServerCenter::cmpInitial(int nSocket, int nCommand, int nSequence, const void *pData)
{
	char *pBody = (char*) ((char *) const_cast<void*>(pData) + sizeof(CMP_HEADER));
	int nType = ntohl(*((int*) pBody));
	_log("[Server Center] Receice CMP Init: type=%d ", nType);

	string strData;
	switch (nType)
	{
	case TYPE_MOBILE_SERVICE:
	case TYPE_POWER_CHARGE_SERVICE:
	case TYPE_SDK_SERVICE:
	case TYPE_TRACKER_SERVICE:
	case TYPE_TRACKER_APPLIENCE:
	case TYPE_TRACKER_TOY:
	case TYPE_TRACKER_IOT:
		strData =
				"{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"54.199.198.94\",\"port\": 6607	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"54.199.198.94\",\"port\": 2307	}]}";
		break;

	}

	if (strData.empty())
	{
		_log("[Server Center] Initial Fail, Can't get initial data Socket FD:%d, Service Type: %d", nSocket, nType);
		sendCommand(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand, STATUS_RSYSERR, nSequence);
	}
	else
	{
		sendCommand(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence,
				strData.c_str());
	}

	return 0;
}

