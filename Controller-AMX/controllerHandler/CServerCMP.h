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
	} AMX_COMMAND;
public:
	CServerCMP(CObject *object);
	virtual ~CServerCMP();
	void broadcastAMXStatus(const char *szStatus);
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);
	void setAmxBusyTimeout(int nSec);

protected:
	void onTimer(int nId);
	int onBind(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onUnbind(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onAmxControl(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onAmxStatus(int nSocket, int nCommand, int nSequence, const void *szBody);

private:
	volatile int mnBusy;
	int mAmxBusyTimeout;
	std::map<int, int> mapClient;
	CObject *mpController;

};
