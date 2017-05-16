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
						strPort = config->getValue("SERVER AUTHENTICATION", "port");
						if(!strPort.empty())
						{
							convertFromString(nPort, strPort);
							if(serverAuth->start(0, nPort, mnMsqKey))
							{
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
	string strToken;
	string
	switch(message.what)
	{
	case amx_control_request: // From CMP Server
		if(!serverAuth->auth(strToken,strId))
		serverAMX->requestAMX(message.strData.c_str());
		break;
	case amx_status_request: // From CMP Server
		serverAMX->requestAMX(message.strData.c_str());
		break;
	case amx_status_response: // From AMX Box
		serverCMP->broadcastAMXStatus(message.strData.c_str());
		break;
	case authentication_response:
		break;
	default:
		_log("[CController] onHandleMessage Unknow what: %d", message.what);
		break;
	}
}
