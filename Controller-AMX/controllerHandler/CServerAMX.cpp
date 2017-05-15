/*
 * CServerAMX.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#include "CServerAMX.h"
#include "config.h"

CServerAMX::CServerAMX(CObject *object)
{
	mpController = object;
}

CServerAMX::~CServerAMX()
{

}

int CServerAMX::onAmxStatus(unsigned long int nSocketFD, const char *szStatus)
{
	Message message;
	message.what = RESPONSE_AMX_STATUS;
	message.strData = szStatus;
	return mpController->sendMessage(message);
}
