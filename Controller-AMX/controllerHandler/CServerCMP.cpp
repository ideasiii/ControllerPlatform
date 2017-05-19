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
#include "utility.h"

using namespace std;

CServerCMP::CServerCMP(CObject *object)
{
	mpController = object;
}

CServerCMP::~CServerCMP()
{

}

int CServerCMP::onBind(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
	mapClient.insert(nSocket);
	_log("[CServerCMP] Socket Client FD:%d Binded", nSocket);
	return TRUE;
}

int CServerCMP::onUnbind(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
	if (mapClient.end() != mapClient.find(nSocket))
	{
		mapClient.erase(nSocket);
		_log("[CServerCMP] Socket Client FD:%d Unbinded", nSocket);
	}
	return TRUE;
}

void CServerCMP::onClientConnect(unsigned long int nSocketFD)
{

}

void CServerCMP::onClientDisconnect(unsigned long int nSocketFD)
{
	if (mapClient.end() != mapClient.find(nSocketFD))
	{
		mapClient.erase(nSocketFD);
		_log("[CServerCMP] Socket Client FD:%d Unbinded", nSocketFD);
	}
}

int CServerCMP::onAmxControl(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	string strBody;
	string strCommand;
	int nStatus;
	AMX_COMMAND amxCommand;

	strBody = reinterpret_cast<const char*>(szBody);

	nStatus = STATUS_RINVJSON;

	_log("[CServerCMP] onAmxControl Body: %s", strBody.c_str());

	if (!strBody.empty())
	{
		JSONObject *jobj = new JSONObject(strBody);
		if (jobj->isValid())
		{
			Message message;
			int nId = getSequence();

			amxCommand.nFunction = jobj->getInt("function");
			amxCommand.nDevice = jobj->getInt("device");
			amxCommand.nControl = jobj->getInt("control");
			amxCommand.strToken = jobj->getString("TOKEN");
			amxCommand.strId = jobj->getString("ID");

			//========= auth token ==================//
			if (amxCommand.strToken.empty() || amxCommand.strId.empty())
			{
				_log("[CServerCMP] onAmxControl Invalid Auth data");
				return response(nSocket, nCommand, nStatus, nSequence, 0);
			}

			message.what = authentication_request;
			message.arg[0] = nId;
			message.strData = format("{\"TOKEN\":\"%s\",\"ID\":\"%s\"}", amxCommand.strToken, amxCommand.strId);
			mpController->sendMessage(message);

			//========= send control command ========//
			strCommand = getAMXControlRequest(amxCommand.nFunction, amxCommand.nDevice, amxCommand.nControl);
			if (!strCommand.empty())
			{
				_log("[CServerCMP] onAmxControl Command: %s", strCommand.c_str());
				nStatus = STATUS_ROK;
				message.what = amx_control_request;
				message.arg[0] = nId;
				message.strData = strCommand;
				mpController->sendMessage(message);
			}
		}
		jobj->release();
		delete jobj;
	}

	return response(nSocket, nCommand, nStatus, nSequence, 0);
}

int CServerCMP::onAmxStatus(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	string strBody;
	string strCommand;
	int nStatus;
	AMX_COMMAND amxCommand;

	strBody = reinterpret_cast<const char*>(szBody);

	nStatus = STATUS_RINVJSON;

	_log("[CServerCMP] onAmxStatus Body: %s", strBody.c_str());

	if (!strBody.empty())
	{
		JSONObject *jobj = new JSONObject(strBody);
		if (jobj->isValid())
		{
			amxCommand.nFunction = jobj->getInt("function");
			amxCommand.nDevice = jobj->getInt("device");
			amxCommand.nStatus = jobj->getInt("request-status");
			strCommand = getAMXStatusRequest(amxCommand.nFunction, amxCommand.nDevice, amxCommand.nStatus);
			if (!strCommand.empty())
			{
				_log("[CServerCMP] onAmxStatus Command: %s", strCommand.c_str());
				nStatus = STATUS_ROK;
				Message message;
				message.what = amx_status_request;
				message.strData = strCommand;
				mpController->sendMessage(message);
			}
		}
		jobj->release();
		delete jobj;
	}

	return response(nSocket, nCommand, nStatus, nSequence, 0);
}

void CServerCMP::broadcastAMXStatus(const char *szStatus)
{
	bool bVol;
	int nLevel;
	int nIndex;
	string strStatus;
	string strLevel;

	if (0 == szStatus)
		return;

	_log("[CServerCMP] broadcastAMXStatus : %s", szStatus);

	nLevel = 0;
	bVol = false;
	strStatus = szStatus;
	nIndex = strStatus.find("_VOL_");

	if ((int) string::npos != nIndex)
	{
		//====== 這是一個聲音狀態 =======//
		bVol = true;
		strLevel = strStatus.substr(nIndex + 5);
		strStatus = strStatus.substr(0, nIndex + 4);
		convertFromString(nLevel, strLevel);
		_log("[CServerCMP] broadcastAMXStatus %s Volumn Level: %d", strStatus.c_str(), nLevel);
	}

	int nId = getAMXStatusResponse(strStatus.c_str());
	if (10000 > nId)
	{
		_log("[CServerCMP] broadcastAMXStatus Invalid status: %s , code:%d", szStatus, nId);
		return;
	}

	JSONObject jobjStatus;
	jobjStatus.put("function", nId / 10000);
	jobjStatus.put("device", (nId % 10000) / 100);
	if (bVol && !strLevel.empty())
	{

		jobjStatus.put("level", nLevel);
	}
	else
		jobjStatus.put("status", (nId % 10000) % 100);

	string strJSON = jobjStatus.toString();
	jobjStatus.release();

	int nRet = 0;
	set<int>::iterator it;
	for (it = mapClient.begin(); it != mapClient.end(); ++it)
	{
		_log("[CServerCMP] broadcastAMXStatus AMX Status: %s to Socket:%d", strJSON.c_str(), *it);
		request(*it, amx_broadcast_status_request, STATUS_ROK, getSequence(), strJSON.c_str());
		if (0 >= nRet)
			break;
	}
}

string CServerCMP::taskName()
{
	return "CServerCMP";
}
