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

#define ID_TIMER_ENQUIRELINK		2311

CServerAuth::CServerAuth(CObject *object) :
		mAuthServer(0), mnEnquuireSeq(-1)
{
	mpController = object;
}

CServerAuth::~CServerAuth()
{

}

int CServerAuth::onBind(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
	closeClient(mAuthServer);
	mAuthServer = nSocket;
	_log("[CServerAuth] onBind Socket: %d", nSocket);
	mnEnquuireSeq = -1;
	setTimer(ID_TIMER_ENQUIRELINK, 1, 3);
	return TRUE;
}

int CServerAuth::onUnbind(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
	mAuthServer = 0;
	_log("[CServerAuth] onUnbind Socket: %d", nSocket);
	mnEnquuireSeq = -1;
	killTimer(ID_TIMER_ENQUIRELINK);
	return TRUE;
}

int CServerAuth::isValid()
{
	return mAuthServer;
}

int CServerAuth::auth(const char *szToken, const char *szID)
{
	string strBody;

	if(mAuthServer && szToken && szID)
	{
		strBody = format("{\"TOKEN\":\"%s\",\"ID\":\"%s\"}", szToken, szID);
		_log("[CServerAuth] auth Body: %s", strBody.c_str());
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
	string strAuth;
	int nAuth;

	if(enquire_link_request == (0xFF & nCommand))
	{
		mnEnquuireSeq = -1;
		_DBG("[CServerAuth] onResponse enquire_link_response Socket: %d", nSocket);
		return nSocket;
	}

	if((authentication_request == (0xFF & nCommand)) && szBody)
	{
		strBody = reinterpret_cast<const char*>(szBody);
		JSONObject *jobj = new JSONObject(strBody);
		if(jobj->isValid())
		{
			strId = jobj->getString("ID");
			strAuth = jobj->getString("AUTH");
			if(!strAuth.empty() && 0 == strAuth.compare("y"))
				nAuth = 1;
			else
				nAuth = 0;
			if(!strId.empty())
			{
				Message message;
				message.what = authentication_response;
				message.arg[0] = nAuth;
				message.strData = strId;
				mpController->sendMessage(message);
			}
		}
		jobj->release();
		delete jobj;
	}
	return nSocket;
}

string CServerAuth::taskName()
{
	return "CServerAuth";
}

void CServerAuth::onTimer(int nId)
{
	_DBG("[CServerAuth] onTimer id: %d", nId);
	if(-1 != mnEnquuireSeq)
	{
		Message message;
		message.what = reboot_request;
		mpController->sendMessage(message);
		_log("[CServerAuth] onTimer Socket: %d Invalid Connected", mAuthServer);
		closeClient(mAuthServer);
		mAuthServer = 0;
		mnEnquuireSeq = -1;
		killTimer(ID_TIMER_ENQUIRELINK);
	}
	else
	{
		if(0 < mAuthServer)
		{
			mnEnquuireSeq = getSequence();
			request(mAuthServer, enquire_link_request, STATUS_ROK, mnEnquuireSeq, 0);
		}
	}
}

void CServerAuth::onClientConnect(unsigned long int nSocketFD)
{
	_log("[CServerAuth] onClientConnect Socket: %d", nSocketFD);
}

void CServerAuth::onClientDisconnect(unsigned long int nSocketFD)
{
	mAuthServer = 0;
	_log("[CServerAuth] onClientDisconnect Socket: %d Unbind", nSocketFD);
}

