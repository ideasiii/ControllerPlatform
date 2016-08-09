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
#include "CObject.h"
#include "common.h"

#define MAX_FUNC_POINT		256

class CCmpHandler;
class CSqliteHandler;
class CThreadHandler;
class CJsonHandler;
class CServerAMX;
class CServerDevice;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	void runEnquireLinkRequest();
	int startSqlite(const int nDBId, const std::string strDB);
	int startServerAMX(const int nPort, const int nMsqId);
	int startServerDevice(const int nPort, const int nMsqId);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();
	void onCMP(int nClientFD, int nDataLen, const void *pData);
	int sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp);
	void ackPacket(int nClientSocketFD, int nCommand, const void * pData);

	/**  Receive CMP Request **/
	int cmpUnknow(int nSocket, int nCommand, int nSequence, const void * pData);
	int cmpBind(int nSocket, int nCommand, int nSequence, const void * pData);
	int cmpUnbind(int nSocket, int nCommand, int nSequence, const void * pData);
	int cmpDeviceControl(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpDeviceState(int nSocket, int nCommand, int nSequence, const void *pData);

	/** Send CMP Request **/
	int cmpPowerPortRequest(int nSocket, std::string strWire, std::string strPort, std::string strState);
	int cmpPowerPortStateRequest(int nSocket, std::string strWire);

	/** Send CMP Response **/
	int cmpResponse(const int nSocket, const int nCommandId, const int nSequence, const char * szData = 0);

	int getControllerSocketFD(std::string strControllerID);
	int getBindSocket(std::list<int> &listValue);
	int cmpEnquireLinkRequest(const int nSocketFD);

private:
	CServerAMX *serverAMX;
	CServerDevice *serverDevice;
	CCmpHandler *cmpParser;
	CSqliteHandler *sqlite;
	CThreadHandler *tdEnquireLink;
	CThreadHandler *tdExportLog;
	std::vector<int> vEnquireLink;

	typedef int (CController::*MemFn)(int, int, int, const void *);
	MemFn cmpRequest[MAX_FUNC_POINT];

};
