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

#include "callback.h"

#define MAX_SIZE		2048

int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

CCmpSignin::CCmpSignin()
{

}

CCmpSignin::~CCmpSignin()
{

}

int CCmpSignin::onSignin(int nSocket, int nCommand, int nSequence, const void *pData)
{

	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);

	char *pBody = (char*) ((char *) const_cast<void*>(pData) + sizeof(CMP_HEADER));
	int nType = ntohl(*((int*) pBody));
	pBody += 4;

	if (isValidStr((const char*) pBody, MAX_SIZE))
	{
		char temp[MAX_SIZE];
		memset(temp, 0, sizeof(temp));
		strcpy(temp, pBody);

		if (0 < strlen(temp))
		{
			_log("[CCmpSignin] onSignin: Sequence: %d Data: %s Socket[%d]", nSequence, temp, nSocket);
			if (mapCallback.end() != mapCallback.find(CB_RUN_MYSQL_SQL))
			{
				(*mapCallback[CB_RUN_MYSQL_SQL])(static_cast<void*>(temp));
			}
		}
	}

	return 0;
}

void CCmpSignin::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

