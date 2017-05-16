/*
 * CServerCMP.h
 *
 *  Created on: 2016撟�8���9�
 *      Author: root
 */

#pragma once

#include <map>

#include "CCmpServer.h"

class CServerCMP: public CCmpServer
{
	typedef struct _AMX_COMMAND
	{
		int nFunction;
		int nDevice;
		int nControl;
		int nStatus;
		std::string strToken;
		std::string strId;
	} AMX_COMMAND;
public:
	CServerCMP(CObject *object);
	virtual ~CServerCMP();
	void broadcastAMXStatus(const char *szStatus);
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);

protected:
	int onBind(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onUnbind(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onAmxControl(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onAmxStatus(int nSocket, int nCommand, int nSequence, const void *szBody);

private:
	std::map<int, int> mapClient;
	CObject *mpController;

};
