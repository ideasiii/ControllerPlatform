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
		_log("Device Server Socket Create Fail");
		return FALSE;
	}

	return TRUE;
}

void CServerDevice::stopServer()
{
	stop();
}

void CServerDevice::onReceive(const int nSocketFD, const void *pData, CBFun cbfun)
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
			"[Server Device Recv]", nSocketFD);

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

//	(*cbfun)(static_cast<void*>(const_cast<char*>("dddddxxxxxx")));
}

int CServerDevice::cmpAmxControl(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("data"))
	{
		_log("[Controller] cmpAmxControl Body: %s", rData["data"].c_str());

		/** get AMX string command **/
		JSONObject jobj(rData["data"].c_str());
		if (jobj.isValid())
		{
			int nFunction = jobj.getInt("function");
			int nDevice = jobj.getInt("device");
			int nControl = jobj.getInt("control");
			string strCommand = getAMXControl(nFunction, nDevice, nControl);
			if (!strCommand.empty())
			{
				sendCommand(nSocket, nCommand, STATUS_ROK, nSequence, true, dynamic_cast<CSocket*>(serverDevice));
				(*mapCallback[CB_AMX_COMMAND])(static_cast<void*>(const_cast<char*>(strCommand.c_str())));
				return TRUE;
			}
		}
		_log("[Controller] cmpAmxControl Fail, Invalid JSON Data Socket FD:%d", nSocket);
		sendCommand(nSocket, nCommand, STATUS_RINVJSON, nSequence, true, dynamic_cast<CSocket*>(serverDevice));
	}

	_log("[Controller] cmpAmxControl Fail, Invalid Body Parameters Socket FD:%d", nSocket);
	sendCommand(nSocket, nCommand, STATUS_RINVBODY, nSequence, true, dynamic_cast<CSocket*>(serverDevice));

	rData.clear();
	return FALSE;
}

void CServerDevice::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

