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

class CCmpHandler;
class CThreadHandler;
class CJsonHandler;
class CServerAMX;
class CServerDevice;
class CSocket;

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);

private:
	int startServerAMX(const char *szIP, const int nPort, const int nMsqId);
	int startServerDevice(const char *szIP, const int nPort, const int nMsqId);
	void stopServer();
	void onAMXCommand(std::string strCommand);
	void onAMXResponseStatus(std::string strStatus);
	void setAMXBusyTimer(int nSec);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

public:
	CCmpHandler *cmpParser;

private:
	CServerAMX *serverAMX;
	CServerDevice *serverDevice;
	CThreadHandler *tdEnquireLink;
	CThreadHandler *tdExportLog;
	//std::vector<int> vEnquireLink;
};
