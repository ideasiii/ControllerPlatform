/*
 * CCmpSignin.cpp
 *
 *  Created on: 2017年3月16日
 *      Author: root
 */

#include "CCmpSignin.h"
#include "packet.h"
#include "utility.h"
#include "common.h"
#include "CObject.h"

#define MAX_SIZE		2048

CCmpSignin::CCmpSignin(CObject *object)
{
	mpController = object;
}

CCmpSignin::~CCmpSignin()
{

}

int CCmpSignin::onSignin(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);

	const char *pBody = reinterpret_cast<const char*>(szBody);

	int nType = ntohl(*((int*) pBody));
	pBody += 4;

	if(isValidStr(pBody, MAX_SIZE))
	{
		Message message;
		message.what = sign_up_request;
		message.strData = pBody;
		mpController->sendMessage(message);
	}
	return TRUE;
}
