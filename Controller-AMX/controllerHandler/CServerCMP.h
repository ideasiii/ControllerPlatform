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
public:
	CServerCMP();
	virtual ~CServerCMP();
	void broadcastAMXStatus(const char *szStatus);
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);
	void setAmxBusyTimeout(int nSec);

protected:
	void onTimer(int nId);
	int onBind(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody);
	int onUnbind(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody);

private:
	volatile int mnBusy;
	int mAmxBusyTimeout;

};
