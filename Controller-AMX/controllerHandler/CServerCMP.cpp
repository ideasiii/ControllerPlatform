/*
 * CServerDevice.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#include <string>
#include "CServerCMP.h"
#include "AMXCommand.h"
#include "common.h"
#include "packet.h"
#include "JSONObject.h"

#define TIMER_ID_AMX_BUSY	666

using namespace std;

CServerCMP::CServerCMP(CObject *object) :
		mnBusy(FALSE), mAmxBusyTimeout(5), mpController(0)
{
	mpController = object;
}

CServerCMP::~CServerCMP()
{

}

int CServerCMP::onBind(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	mapClient[nSocket] = nSocket;
	_log("[CServerCMP] Socket Client FD:%d Binded", nSocket);
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
	return TRUE;
}

int CServerCMP::onUnbind(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	if(mapClient.end() != mapClient.find(nSocket))
	{
		mapClient.erase(nSocket);
		_log("[Server Device] Socket Client FD:%d Unbinded", nSocket);
	}
	return TRUE;
}

int CServerCMP::onAmxControl(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	string strBody;
	string strCommand;
	AMX_COMMAND amxCommand;

	strBody = reinterpret_cast<const char*>(szBody);

	if(!strBody.empty())
	{
		JSONObject *jobj = new JSONObject(strBody);
		if(jobj->isValid())
		{
			amxCommand.nFunction = jobj->getInt("function");
			amxCommand.nDevice = jobj->getInt("device");
			amxCommand.nControl = jobj->getInt("control");
			strCommand = getAMXControlRequest(amxCommand.nFunction, amxCommand.nDevice, amxCommand.nControl);
			if(!strCommand.empty())
			{
				response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
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
					(*mapCallback[CB_AMX_COMMAND_CONTROL])(static_cast<void*>(const_cast<char*>(strCommand.c_str())));

					setTimer(TIMER_ID_AMX_BUSY, mAmxBusyTimeout, 0);
				}
			}
			else
			{
				response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
			}
		}
		else
		{
			response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
		}

		jobj->release();
		delete jobj;

	}
	else
	{
		response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
	}

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
					(*mapCallback[CB_AMX_COMMAND_CONTROL])(static_cast<void*>(const_cast<char*>(strCommand.c_str())));

					setTimer(TIMER_ID_AMX_BUSY, mAmxBusyTimeout, 0);
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

int CServerCMP::onAmxStatus(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	int nStatus = STATUS_RINVBODY;

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
			string strCommand = getAMXStatusRequest(nFunction, nDevice, nRqStatus);
			if(!strCommand.empty())
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
	sendPacket(dynamic_cast<CSocket*>(serverDevice), nSocket, generic_nack | nCommand, nStatus, nSequence, 0);
	rData.clear();
	return FALSE;
}

void CServerCMP::broadcastAMXStatus(const char *szStatus)
{
	int nId = AMX_STATUS_RESP[szStatus];
	if(10000 > nId)
	{
		_log("[Server Device] Invalid status: %s , code:%d", szStatus, nId);
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

void CServerCMP::onTimer(int nId)
{
	if(TIMER_ID_AMX_BUSY == nId)
	{
		mnBusy = FALSE;
		_log("[Server Device] AMX Controller Busy set to False");
	}
}

void CServerCMP::setAmxBusyTimeout(int nSec)
{
	mAmxBusyTimeout = nSec;
	_log("[Server Device] Set AMX Busy Timeout: %d seconds", mAmxBusyTimeout);
}
