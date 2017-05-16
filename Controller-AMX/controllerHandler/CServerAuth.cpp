/*
 * CServerAuth.cpp
 *
 *  Created on: 2017年5月16日
 *      Author: root
 */

#include <string>
#include "CServerAuth.h"
#include "common.h"
#include "utility.h"
#include "packet.h"
#include "JSONObject.h"

using namespace std;

CServerAuth::CServerAuth(CObject *object) :
		mAuthServer(0)
{
	mpController = object;
}

CServerAuth::~CServerAuth()
{

}

int CServerAuth::onBind(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	mAuthServer = nSocket;
	_log("[CServerAuth] onBind Socket: %d", nSocket);
	return TRUE;
}

int CServerAuth::onUnbind(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	mAuthServer = 0;
	_log("[CServerAuth] onUnbind Socket: %d", nSocket);
	return TRUE;
}

int CServerAuth::auth(const char *szToken, const char *szIp)
{
	string strBody;

	strBody = format("{\"TOKEN\":\"%s\",\"ID\":\"%s\"}", szToken, szIP);

	if(mAuthServer)
	{
		mapAuth[szIp] = 0;
		request(mAuthServer, authentication_request, STATUS_ROK, getSequence(), strBody.c_str());
	}
	else
	{
		_log("[CServerAuth] auth Auth Server not Bind");
		return FALSE;
	}
	return TRUE;
}

int CServerAuth::onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody)
{
	string strBody;
	string strId;

	if(authentication_response == nCommand && szBody)
	{
		strBody = reinterpret_cast<const char*>(szBody);
		JSONObject *jobj = new JSONObject(strBody);
		if(jobj->isValid())
		{
			strId = jobj->getString("ID");
			if(!strId.empty())
			{
				if(mapAuth.end() != mapAuth.find(strId))
				{
					mapAuth[strId] = 1;
					Message message;
					message.what = authentication_response;
					message.strData = strId;
					mpController->sendMessage(message);
				}
			}
		}
		jobj->release();
		delete jobj;
	}
	return nSocket;
}

