/*
 * CServerDevice.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#include "CServerDevice.h"
#include "IReceiver.h"
#include "event.h"
#include "packet.h"
#include "common.h"
#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "JSONObject.h"
#include "AMXCommand.h"

static CServerDevice * serverDevice = 0;

CServerDevice::CServerDevice() :
		CSocketServer(), cmpParser(CCmpHandler::getInstance())
{
	mapFunc[amx_control_request] = &CServerDevice::cmpAmxControl;
	mapFunc[amx_status_request] = &CServerDevice::cmpAmxStatus;
}

CServerDevice::~CServerDevice()
{
	stop();
}

CServerDevice * CServerDevice::getInstance()
{
	if (0 == serverDevice)
	{
		serverDevice = new CServerDevice();
	}
	return serverDevice;
}

int CServerDevice::startServer(const int nPort, const int nMsqId)
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

	if (FAIL == start( AF_INET, NULL, nPort))
	{
		_log("[Server Device] Socket Create Fail");
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
			"[Server Device] Recv ", nSocketFD);

	if (cmpParser->isAckPacket(cmpHeader.command_id))
	{
		return;
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if (0x000000FF < cmpHeader.command_id || 0x00000000 >= cmpHeader.command_id || mapFunc.end() == iter)
	{
		sendCommand(nSocketFD, cmpHeader.command_id, STATUS_RINVCMDID, cmpHeader.sequence_number, true,
				dynamic_cast<CSocket*>(serverDevice));
		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);

}

int CServerDevice::cmpAmxControl(int nSocket, int nCommand, int nSequence, const void *pData)
{
	int nStatus = STATUS_RINVBODY;

	CDataHandler<string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("data"))
	{
		_log("[Server Device] AMX Control Request Body: %s", rData["data"].c_str());

		/** get AMX string command **/
		JSONObject jobj(rData["data"].c_str());
		if (jobj.isValid())
		{
			int nFunction = jobj.getInt("function");
			int nDevice = jobj.getInt("device");
			int nControl = jobj.getInt("control");
			string strCommand = getAMXControlRequest(nFunction, nDevice, nControl);
			if (!strCommand.empty())
			{
				nStatus = STATUS_ROK;
				(*mapCallback[CB_AMX_COMMAND_CONTROL])(static_cast<void*>(const_cast<char*>(strCommand.c_str())));
			}
			else
			{
				_log("[Server Device] AMX Control Request Fail, Invalid JSON Data, No AMX Command Socket FD:%d",
						nSocket);
				nStatus = STATUS_RINVJSON;
			}
		}
		else
		{
			_log("[Server Device] AMX Control Request Fail, Invalid JSON Data Socket FD:%d", nSocket);
			nStatus = STATUS_RINVJSON;
		}
	}
	else
	{
		_log("[Server Device] AMX Control Request Fail, Invalid Body Parameters Socket FD:%d", nSocket);
	}
	sendCommand(nSocket, nCommand, nStatus, nSequence, true, dynamic_cast<CSocket*>(serverDevice));
	rData.clear();
	return FALSE;
}

/**
 * AMX Status Request
 * {
 "function": 1,
 "device": 0,
 " request-status ": 1
 }
 */
int CServerDevice::cmpAmxStatus(int nSocket, int nCommand, int nSequence, const void *pData)
{
	int nStatus = STATUS_RINVBODY;

	CDataHandler<string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("data"))
	{
		_log("[Server Device] AMX Status Request Body: %s", rData["data"].c_str());

		/** get AMX string command **/
		JSONObject jobj(rData["data"].c_str());
		if (jobj.isValid())
		{
			int nFunction = jobj.getInt("function");
			int nDevice = jobj.getInt("device");
			int nRqStatus = jobj.getInt("request-status");
			string strCommand = getAMXStatusRequest(nFunction, nDevice, nRqStatus);
			if (!strCommand.empty())
			{
				nStatus = STATUS_ROK;
				(*mapCallback[CB_AMX_COMMAND_STATUS])(static_cast<void*>(const_cast<char*>(strCommand.c_str())));
			}
			else
			{
				_log("[Server Device] cmpAmxControl Fail, Invalid JSON Data, No AMX Command Socket FD:%d", nSocket);
				nStatus = STATUS_RINVJSON;
			}
		}
		else
		{
			_log("[Server Device] cmpAmxControl Fail, Invalid JSON Data Socket FD:%d", nSocket);
			nStatus = STATUS_RINVJSON;
		}
	}
	else
	{
		_log("[Server Device] cmpAmxControl Fail, Invalid Body Parameters Socket FD:%d", nSocket);
	}
	sendCommand(nSocket, nCommand, nStatus, nSequence, true, dynamic_cast<CSocket*>(serverDevice));
	rData.clear();
	return FALSE;
}

void CServerDevice::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

void CServerDevice::addClient(const int nSocketFD)
{
	mapClient[nSocketFD] = nSocketFD;
	_log("[Server Device] Socket Client FD:%d Connected", nSocketFD);
}

void CServerDevice::deleteClient(const int nSocketFD)
{
	mapClient.erase(nSocketFD);
	_log("[Server Device] Socket Client FD:%d Closed", nSocketFD);
}

/**
 * AMX Status Response
 {
 “function”:1,
 “device”:0,
 “status”:1
 }
 *
 */
void CServerDevice::broadcastAMXStatus(string strStatus)
{
	int nId = AMX_STATUS_RESP[strStatus];
	if (10000 > nId)
		return;

	int nResult = 0;
	JSONObject jobjStatus;
	jobjStatus.put("function", nId / 10000);
	jobjStatus.put("device", (nId % 10000) / 100);
	jobjStatus.put("status", (nId % 10000) % 100);

	string strJSON = jobjStatus.toString();
	jobjStatus.release();

	map<int, int>::iterator it;
	for (it = mapClient.begin(); it != mapClient.end(); ++it)
	{
//		nResult = cmpResponse(it->first, amx_status_response, getSequence(),strJSON.c_str());
		_log("[Server Device] Broadcast AMX Status: %s to Socket:%d", strJSON.c_str(), it->first);
	}
}

int CServerDevice::cmpResponse(const int nSocket, const int nCommandId, const int nSequence, const char * szData)
{
	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	void *pHeader = &packet.cmpHeader;
	char *pIndex = packet.cmpBody.cmpdata;

	memset(&packet, 0, sizeof(CMP_PACKET));

	cmpParser->formatHeader(nCommandId, STATUS_ROK, nSequence, &pHeader);
	memcpy(pIndex, szData, strlen(szData));
	pIndex += strlen(szData);
	nBody_len += strlen(szData);
	memcpy(pIndex, "\0", 1);
	pIndex += 1;
	nBody_len += 1;

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);

//	nRet = cmpServer->socketSend(nSocket, &packet, nTotal_len);
	printPacket(nCommandId, STATUS_ROK, nSequence, nRet, "[Controller] cmpResponse", nSocket);

	string strLog;
	if (0 >= nRet)
	{
		_log("[Controller] cmpResponse Fail socket: %d", nSocket);
	}

	return nRet;
}

