/*
 * CServerAuth.h
 *
 *  Created on: 2017年5月16日
 *      Author: Jugo
 */

#pragma once

#include <map>
#include "CCmpServer.h"

class CServerAuth: public CCmpServer
{
public:
	CServerAuth(CObject *object);
	virtual ~CServerAuth();
	int auth(const char *szToken, const char *szIp);

protected:
	int onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody);
	int onBind(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onUnbind(int nSocket, int nCommand, int nSequence, const void *szBody);

private:
	CObject *mpController;
	unsigned long int mAuthServer;
	std::map<std::string, int> mapAuth;
};
