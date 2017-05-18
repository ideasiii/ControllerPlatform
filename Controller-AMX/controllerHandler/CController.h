/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>
#include "CApplication.h"

class CServerAMX;
class CServerCMP;
class CServerAuth;

class CController: public CApplication
{
	typedef struct _AMX_CTRL_AUTH
	{
		std::string strToken;
		std::string strId;
		std::string strCommand;
	} AMX_CTRL_AUTH;

public:
	explicit CController();
	virtual ~CController();

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);

private:
	int mnMsqKey;
	CServerAMX *serverAMX;
	CServerCMP *serverCMP;
	CServerAuth *serverAuth;
	std::map<int, AMX_CTRL_AUTH> mapCtrlAuth;
};
