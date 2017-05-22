/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#include "CController.h"
#include "common.h"
#include "CConfig.h"
#include "CServerAMX.h"
#include "CServerCMP.h"
#include "CServerAuth.h"
#include "utility.h"
#include "event.h"
#include "packet.h"
#include "JSONObject.h"

using namespace std;

CController::CController() :
		mnMsqKey(-1), serverAMX(0), serverCMP(0), serverAuth(0)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	serverAMX = new CServerAMX(this);
	serverCMP = new CServerCMP(this);
	serverAuth = new CServerAuth(this);
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_AMX;
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nSecond;
	int nRet;
	int nPort;
	string strPort;
	CConfig *config;
	string strConfPath;

	nRet = FALSE;
	strConfPath = reinterpret_cast<const char*>(szConfPath);
	if(strConfPath.empty())
		return nRet;

	_log("[CController] onInitial Config File: %s", strConfPath.c_str());

	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		strPort = config->getValue("SERVER AMX", "port");
		if(!strPort.empty())
		{
			convertFromString(nPort, strPort);
			if(serverAMX->start(0, nPort, mnMsqKey))
			{
				strPort = config->getValue("SERVER DEVICE", "port");
				if(!strPort.empty())
				{
					convertFromString(nPort, strPort);
					if(serverCMP->start(0, nPort, mnMsqKey))
					{
						_log("[CController] onInitial CMP Server Start! Port: %d", nPort);
						strPort = config->getValue("SERVER AUTHENTICATION", "port");
						if(!strPort.empty())
						{
							convertFromString(nPort, strPort);
							if(serverAuth->start(0, nPort, mnMsqKey))
							{
								_log("[CController] onInitial Auth Server Start! Port: %d", nPort);
								nRet = TRUE;
							}
						}
					}
				}
			}
		}

	}
	delete config;
	return nRet;
}

int CController::onFinish(void* nMsqKey)
{
	if(serverAMX)
	{
		serverAMX->stop();
		delete serverAMX;
		serverAMX = 0;
	}

	if(serverCMP)
	{
		serverCMP->stop();
		delete serverCMP;
		serverCMP = 0;
	}

	if(serverAuth)
	{
		serverAuth->stop();
		delete serverAuth;
		serverAuth = 0;
	}
	return TRUE;
}

void CController::onHandleMessage(Message &message)
{

	switch(message.what)
	{
	case authentication_request:
	{
		JSONObject *jobj = new JSONObject(message.strData);
		if(jobj->isValid())
		{
			string strToken = jobj->getString("TOKEN");
			string strId = jobj->getString("ID");
			if(!strToken.empty() && !strId.empty())
			{
				AMX_CTRL_AUTH ctrlAuth;
				ctrlAuth.strId = strId;
				ctrlAuth.strToken = strToken;
				mapCtrlAuth[message.arg[0]] = ctrlAuth;
				serverAuth->auth(strToken.c_str(), strId.c_str());
			}
		}
		jobj->release();
		delete jobj;
	}
		break;
	case amx_control_request: // From CMP Server
		if(!serverAuth->isValid())
			serverAMX->requestAMX(message.strData.c_str());
		else
		{
			map<int, AMX_CTRL_AUTH>::const_iterator it;
			it = mapCtrlAuth.find(message.arg[0]);
			if(mapCtrlAuth.end() == it)
			{
				serverAMX->requestAMX(message.strData.c_str());
			}
			else
			{
				AMX_CTRL_AUTH ctrlAuth;
				ctrlAuth.strId = it->second.strId;
				ctrlAuth.strToken = it->second.strToken;
				ctrlAuth.strCommand = message.strData;
				mapCtrlAuth[message.arg[0]] = ctrlAuth;
			}
		}
		break;
	case amx_status_request: // From CMP Server
		_log("[CController] onHandleMessage amx_status_request from socket: %d data: %s", message.arg[0],
						message.strData.c_str());
		serverAMX->requestAMX(message.strData.c_str());
		//=================== broadcast volum dummy ==================//
		//serverCMP->broadcastAMXStatus("STATUS_INPUT5_VOL_-13");
		//=================== dummy end ==============================//
		break;
	case amx_status_response: // From AMX Box
		_log("[CController] onHandleMessage amx_status_response from socket: %d data: %s", message.arg[0],
				message.strData.c_str());
		serverCMP->broadcastAMXStatus(message.strData.c_str());
		break;
	case authentication_response:
	{
		string strId = message.strData;
		string strCommand;
		int nId = -1;
		map<int, AMX_CTRL_AUTH>::const_iterator it;
		for(it = mapCtrlAuth.begin(); mapCtrlAuth.end() != it; ++it)
		{
			if(0 == strId.compare(it->second.strId))
			{
				nId = it->first;
				strCommand = it->second.strCommand;
				break;
			}
		}

		if(-1 != nId)
		{
			if(1 == message.arg[0] && !strCommand.empty())
			{
				serverAMX->requestAMX(strCommand.c_str());
			}
			mapCtrlAuth.erase(nId);
		}
	}
		break;
	default:
		_log("[CController] onHandleMessage Unknow what: %d", message.what);
		break;
	}
}
