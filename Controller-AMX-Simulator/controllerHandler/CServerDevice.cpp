/*
 * CServerDevice.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#include "CServerDevice.h"
#include "IReceiver.h"
#include "event.h"
#include "common.h"
#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "JSONObject.h"
#include "AMXCommand.h"
#include "ICallback.h"
#include "CTimer.h"
#include "packet.h"

using namespace std;

#define TIMER_ID_AMX_BUSY	666

static CServerDevice * serverDevice = 0;

void OnTimer(int param)
{
	serverDevice->onTimer(param);
}

CServerDevice::CServerDevice() :
		CSocketServer(), cmpParser(CCmpHandler::getInstance()), mnBusy(FALSE), mAmxBusyTimeout(5)
{
	mapFunc[amx_control_request] = &CServerDevice::cmpAmxControl;
	mapFunc[amx_status_request] = &CServerDevice::cmpAmxStatus;
	mapFunc[bind_request] = &CServerDevice::cmpBind;
	mapFunc[unbind_request] = &CServerDevice::cmpUnbind;
}

CServerDevice::~CServerDevice()
{
	stop();
	mapClient.clear();
}

CServerDevice * CServerDevice::getInstance()
{
	if(0 == serverDevice)
	{
		serverDevice = new CServerDevice();
	}
	return serverDevice;
}

int CServerDevice::startServer(string strIP, const int nPort, const int nMsqId)
{
	if(0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if(0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	setPacketConf(PK_CMP, PK_MSQ);

	const char* cszAddr = NULL;
	if(!strIP.empty())
		cszAddr = strIP.c_str();

	if(FAIL == start(AF_INET, cszAddr, nPort))
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

	if(0 == pData)
		return;

	pPacket = (char*) const_cast<void*>(pData);
	memset(&cmpHeader, 0, sizeof(CMP_HEADER));

	cmpHeader.command_id = cmpParser->getCommand(pPacket);
	cmpHeader.command_length = cmpParser->getLength(pPacket);
	cmpHeader.command_status = cmpParser->getStatus(pPacket);
	cmpHeader.sequence_number = cmpParser->getSequence(pPacket);

	printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
			"[Server Device] Recv ", nSocketFD);

	if(cmpParser->isAckPacket(cmpHeader.command_id))
	{
		return;
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if(0x000000FF < cmpHeader.command_id || 0x00000000 >= cmpHeader.command_id || mapFunc.end() == iter)
	{
		sendPacket(dynamic_cast<CSocket*>(serverDevice), nSocketFD, generic_nack | cmpHeader.command_id,
		STATUS_RINVCMDID, cmpHeader.sequence_number, 0);
		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);

}

int CServerDevice::cmpBind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	sendPacket(dynamic_cast<CSocket*>(serverDevice), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, 0);
	mapClient[nSocket] = nSocket;
	_log("[Server Device] Socket Client FD:%d Binded", nSocket);
	return TRUE;
}

int CServerDevice::cmpUnbind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	sendPacket(dynamic_cast<CSocket*>(serverDevice), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, 0);
	deleteClient(nSocket);
	return TRUE;
}

int CServerDevice::cmpAmxControl(int nSocket, int nCommand, int nSequence, const void *pData)
{
	int nStatus = STATUS_RINVBODY;

	CDataHandler<string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if(0 < nRet && rData.isValidKey("data"))
	{
		_log("[Server Device] AMX Control Request Body: %s", rData["data"].c_str());

		/** get AMX string command **/
		JSONObject jobj(rData["data"].c_str());
		if(jobj.isValid())
		{
			int nFunction = jobj.getInt("function");
			int nDevice = jobj.getInt("device");
			int nControl = jobj.getInt("control");
			string strTOKEN = jobj.getString("TOKEN");
			string strID = jobj.getString("ID");

			string strCommand = getAMXControlRequest(nFunction, nDevice, nControl);
			if(!strCommand.empty())
			{
				if(mnBusy)
				{
					nStatus = STATUS_RSYSBUSY;
					_log("[Server Device] AMX Control Request Fail, System Busy, Socket FD:%d", nSocket);
				}
				else
				{
					mnBusy = TRUE;
					_log("[Server Device] AMX Controller Busy set to True");
					nStatus = STATUS_ROK;
					//	(*mapCallback[CB_AMX_COMMAND_CONTROL])(static_cast<void*>(const_cast<char*>(strCommand.c_str())));
					SetTimer(TIMER_ID_AMX_BUSY, mAmxBusyTimeout, 0, OnTimer);
				}
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
	sendPacket(dynamic_cast<CSocket*>(serverDevice), nSocket, generic_nack | nCommand, nStatus, nSequence, 0);
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
	string strCommand;

	CDataHandler<string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if(0 < nRet && rData.isValidKey("data"))
	{
		_log("[Server Device] AMX Status Request Body: %s", rData["data"].c_str());

		/** get AMX string command **/
		JSONObject jobj(rData["data"].c_str());
		if(jobj.isValid())
		{
			int nFunction = jobj.getInt("function");
			int nDevice = jobj.getInt("device");
			int nRqStatus = jobj.getInt("request-status");
			string strTOKEN = jobj.getString("TOKEN");
			string strID = jobj.getString("ID");

			strCommand = getAMXStatusRequest(nFunction, nDevice, nRqStatus);
			if(!strCommand.empty())
			{
				nStatus = STATUS_ROK;
				//	(*mapCallback[CB_AMX_COMMAND_STATUS])(static_cast<void*>(const_cast<char*>(strCommand.c_str())));
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
	sendPacket(dynamic_cast<CSocket*>(serverDevice), nSocket, generic_nack | nCommand, nStatus, nSequence, 0);
	rData.clear();

	//========== boardcast status ===============//
	if(STATUS_ROK == nStatus)
	{
		string strStatus = getAMXCurrentStatus(strCommand.c_str());
		broadcastAMXStatus(strStatus);
	}
	return FALSE;
}

void CServerDevice::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

void CServerDevice::addClient(const int nSocketFD)
{
	_log("[Server Device] Socket Client FD:%d Connected", nSocketFD);
}

void CServerDevice::deleteClient(const int nSocketFD)
{
	if(mapClient.end() != mapClient.find(nSocketFD))
	{
		mapClient.erase(nSocketFD);
		_log("[Server Device] Socket Client FD:%d Unbinded", nSocketFD);
	}
	_log("[Server Device] Socket Client FD:%d Disconnected", nSocketFD);
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
	if(strStatus.empty())
		return;

	int nId = AMX_STATUS_RESP[strStatus];
	if(10000 > nId)
	{
		_log("[Server Device] Invalid status: %s , code:%d", strStatus.c_str(), nId);
		return;
	}

	JSONObject jobjStatus;
	jobjStatus.put("function", nId / 10000);
	jobjStatus.put("device", (nId % 10000) / 100);
	jobjStatus.put("status", (nId % 10000) % 100);

	string strJSON = jobjStatus.toString();
	jobjStatus.release();

	int nRet = 0;
	map<int, int>::iterator it;
	for(it = mapClient.begin(); it != mapClient.end(); ++it)
	{
		_log("[Server Device] Broadcast AMX Status: %s to Socket:%d", strJSON.c_str(), it->first);
		nRet = sendPacket(dynamic_cast<CSocket*>(serverDevice), it->first, amx_broadcast_status_request, STATUS_ROK,
				getSequence(), strJSON.c_str());
		if(0 >= nRet)
			break;
	}
}

void CServerDevice::onTimer(int nId)
{
	if(TIMER_ID_AMX_BUSY == nId)
	{
		mnBusy = FALSE;
		_log("[Server Device] AMX Controller Busy set to False");
	}
}

void CServerDevice::setAmxBusyTimeout(int nSec)
{
	mAmxBusyTimeout = nSec;
	_log("[Server Device] Set AMX Busy Timeout: %d seconds", mAmxBusyTimeout);
}
