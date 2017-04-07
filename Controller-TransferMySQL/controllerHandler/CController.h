/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"

class CTransferUser;
class CTransferTracker;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int start();
	int stop();
	void setPsql(const char *szHost, const char *szPort, const char *szDB, const char *szUser, const char *szPassword);
	void setMysql(const char *szHost, const char *szPort, const char *szDB, const char *szUser, const char *szPassword);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
	void onTimer(int nId);

private:
	explicit CController();

private:
	volatile int mnBusy;
	CTransferUser *transUser;
	CTransferTracker *transTracker;

};

