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
#include "common.h"
#include "packet.h"

class CCmpHandler;
class CSqliteHandler;
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
	void runEnquireLinkRequest();
	int startSqlite(const int nDBId, const std::string strDB);
	int startServerAMX(const int nPort, const int nMsqId);
	int startServerDevice(const int nPort, const int nMsqId);
	void stopServer();
	void onAMXCommand(string strCommand);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();
	int sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp, CSocket *socket);

	/**  Receive CMP Request **/
	int cmpBind(int nSocket, int nCommand, int nSequence, const void * pData);
	int cmpUnbind(int nSocket, int nCommand, int nSequence, const void * pData);
	int cmpAmxControl(int nSocket, int nCommand, int nSequence, const void *pData);

	/** Send CMP Response **/
	int cmpResponse(const int nSocket, const int nCommandId, const int nSequence, const char * szData = 0);

	int getControllerSocketFD(std::string strControllerID);
	int getBindSocket(std::list<int> &listValue);
	int cmpEnquireLinkRequest(const int nSocketFD);
	void onReceiveDevice(const int nSocketFD, const void *pData);
	void onReceiveAMX(const int nSocketFD, char * pCommand);

private:
	CServerAMX *serverAMX;
	CServerDevice *serverDevice;
	CCmpHandler *cmpParser;
	CSqliteHandler *sqlite;
	CThreadHandler *tdEnquireLink;
	CThreadHandler *tdExportLog;
	std::vector<int> vEnquireLink;

	typedef int (CController::*MemFn)(int, int, int, const void *);
//	MemFn cmpRequest[MAX_COMMAND];
	std::map<int, MemFn> mapFunc;

};
