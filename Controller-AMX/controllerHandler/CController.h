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
#include "CObject.h"

class CCmpHandler;
class CThreadHandler;
class CJsonHandler;
class CServerAMX;
class CServerDevice;
class CSocket;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int startServerAMX(std::string strIP, const int nPort, const int nMsqId);
	int startServerDevice(std::string strIP, const int nPort, const int nMsqId);
	void stopServer();
	void onAMXCommand(std::string strCommand);
	void onAMXResponseStatus(std::string strStatus);
	void setAMXBusyTimer(int nSec);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();

public:
	CCmpHandler *cmpParser;

private:
	CServerAMX *serverAMX;
	CServerDevice *serverDevice;
	CThreadHandler *tdEnquireLink;
	CThreadHandler *tdExportLog;
	std::vector<int> vEnquireLink;
};
