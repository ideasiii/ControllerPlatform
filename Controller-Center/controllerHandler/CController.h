/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"

class CServerCenter;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int startServerCenter(const char* szIP, const int nPort, const int nMsqId);
	void stopServer();

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();

private:
	CServerCenter *serverCenter;
};
