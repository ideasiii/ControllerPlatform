/*
 * CServerAMX.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#include "CServerAMX.h"
#include "packet.h"

CServerAMX::CServerAMX(CObject *object) :
		mAmxBox(0)
{
	mpController = object;
}

CServerAMX::~CServerAMX()
{

}

int CServerAMX::onAmxStatus(unsigned long int nSocketFD, const char *szStatus)
{
	Message message;
	message.what = amx_status_response;
	message.strData = szStatus;
	return mpController->sendMessage(message);
}

void CServerAMX::onClientConnect(unsigned long int nSocketFD)
{
	mAmxBox = nSocketFD;
	_log("[CServerAMX] onClientConnect Socket: %d", nSocketFD);
}

void CServerAMX::onClientDisconnect(unsigned long int nSocketFD)
{
	mAmxBox = 0;
	_log("[CServerAMX] onClientDisconnect Socket: %d", nSocketFD);
}

int CServerAMX::requestAMX(const char *szCommand)
{
	if(!mAmxBox)
	{
		_log("[CServerAMX] requestAMX Error!! AMX not Connected");
		return -1;
	}
	return request(mAmxBox, szCommand);
}
