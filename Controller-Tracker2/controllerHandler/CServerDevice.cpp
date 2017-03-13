#include "CServerDevice.h"
#include "IReceiver.h"
#include "event.h"
#include "common.h"
#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "iCommand.h"
#include "JSONObject.h"
#include "packet.h"

static CServerDevice * serverDevice = 0;

CServerDevice::CServerDevice() :
		CSocketServer(), cmpParser(CCmpHandler::getInstance())
{
	mapFunc[access_log_request] = &CServerDevice::cmpAccessLogRequest;

}

CServerDevice::~CServerDevice()
{
	stop();
	mapClient.clear();
}

CServerDevice * CServerDevice::getInstance()
{
	if (0 == serverDevice)
	{
		serverDevice = new CServerDevice();
	}
	return serverDevice;
}

int CServerDevice::startServer(string strIP, const int nPort, const int nMsqId)
{
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if (0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	setPacketConf(PK_CMP, PK_MSQ);

	const char* cszAddr = NULL;
	if (!strIP.empty())
	{
		cszAddr = strIP.c_str();
	}

	if (FAIL == start(AF_INET, cszAddr, nPort))
	{
		_log("[CServerDevice] Socket Create Fail");
		return FALSE;
	}

	return TRUE;
}

void CServerDevice::stopServer()
{
	stop();
}

void CServerDevice::onReceive(const int nSocketFD, const void *pData)
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
			"[CServerDevice] Recv ", nSocketFD);

	if (cmpParser->isAckPacket(cmpHeader.command_id))
	{
		return;
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if (0x000000FF < cmpHeader.command_id || 0x00000000 >= cmpHeader.command_id || mapFunc.end() == iter)
	{
		sendPacket(dynamic_cast<CSocket*>(serverDevice), nSocketFD, generic_nack | cmpHeader.command_id,
		STATUS_RINVCMDID, cmpHeader.sequence_number, 0);

		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);

}

int CServerDevice::cmpAccessLogRequest(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CServerDevice]cmpAccessLogRequest");

	CDataHandler<string> rData;

	if (paseBody(pData, rData) > 0)
	{
		//mongoDB
		_log("[CServerDevice] call back mongo client@@");
		(*mapCallback[CB_CONTROLLER_MONGODB_COMMAND])(static_cast<void*>(const_cast<CDataHandler<string> *>(&rData)));

		//mysql

	}
	else
	{
		_log("[CServerDevice]Error while covert access log!");

	}

	return TRUE;
}

void CServerDevice::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

void CServerDevice::addClient(const int nSocketFD)
{
	_log("[Server Device] Socket Client FD:%d Connected", nSocketFD);

	mapClient[nSocketFD] = nSocketFD;
}

void CServerDevice::deleteClient(const int nSocketFD)
{
	if (mapClient.end() != mapClient.find(nSocketFD))
	{
		mapClient.erase(nSocketFD);
		_log("[Server Device] Socket Client FD:%d Disconnected", nSocketFD);
	}

}

int CServerDevice::paseBody(const void *pData, CDataHandler<string> &rData)
{
	int nStrLen = 0;
	int nTotalLen = 0;
	int nBodyLen = 0;
	int nType = 0;
	char * pBody;
	char temp[MAX_SIZE];

	nTotalLen = getLength(pData);

	_DBG("[CServerDevice] sockect total Length: %d", nTotalLen);

	nBodyLen = nTotalLen - sizeof(CMP_HEADER);

	if (0 < nBodyLen)
	{
		pBody = (char*) ((char *) const_cast<void*>(pData) + sizeof(CMP_HEADER));
		nType = ntohl(*((int*) pBody));
		rData.setData("type", ConvertToString(nType));

		pBody += 4;
		if (isValidStr((const char*) pBody, MAX_SIZE))
		{
			memset(temp, 0, sizeof(temp));
			strcpy(temp, pBody);
			rData.setData("data", temp);
			nStrLen = strlen(temp);
			++nStrLen;
			pBody += nStrLen;
		}
		else
		{
			return -1;
		}
	}
	return 1;
}

int CServerDevice::getLength(const void *pData)
{
	int nLength;
	CMP_HEADER *pHeader;
	pHeader = (CMP_HEADER *) pData;
	nLength = ntohl(pHeader->command_length);
	return nLength;
}

