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
	int auth(const char *szToken, const char *szID);
	int isValid();

protected:
	void onTimer(int nId);
	int onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody);
	int onBind(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onUnbind(int nSocket, int nCommand, int nSequence, const void *szBody);
	std::string taskName();

private:
	CObject *mpController;
	unsigned long int mAuthServer;
};

//=========================封包說明=========================================//
// auth request:
// 		CMP HEADER command id: authentication_request
//		BODY: {
//				"TOKEN":"????",
//				"ID":"????"
//			  }
//
//auth response:
//		CMP HEADER command id: authentication_response
//      BODY: {
//				"ID":"???",  <---- value要跟request的ID value相同，回傳比對用
//				"AUTH":"y"   <---- "y":認証OK，"n":非法入侵
//			  }
//
//========================================================================//
