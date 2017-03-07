/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"

class CServerDevice;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int startServerDevice(const char *szIP, const int nPort, const int nMsqId);
	void stopServer();
	void setAMXBusyTimer(int nSec);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();

private:
	CServerDevice *serverDevice;

};
