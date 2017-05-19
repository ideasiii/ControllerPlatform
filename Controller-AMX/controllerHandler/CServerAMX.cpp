/*
 * CServerAMX.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#include "CServerAMX.h"
#include "packet.h"
#include "utility.h"

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
	if (szStatus)
	{
		Message message;
		message.what = amx_status_response;
		message.strData = szStatus;
		return mpController->sendMessage(message);
	}
	return FALSE;
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
	if (!mAmxBox)
	{
		_log("[CServerAMX] requestAMX Error!! AMX not Connected");
		return -1;
	}
	string strAmxCmd = format("%s\n", szCommand);
	return request(mAmxBox, strAmxCmd.c_str());
}

string CServerAMX::taskName()
{
	return "CServerAMX";
}
