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

class CSocketServer;
class CSocketClient;
class CCmpHandler;
class CSqliteHandler;
class CJsonHandler;
class CRdmOperate;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int startServer(const int nPort, const int nMsqId);
	void stopServer();
	void runEnquireLinkRequest();
	int startSqlite(const int nDBId, const std::string strDB);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();
	void onCMP(int nClientFD, int nDataLen, const void *pData);
	int sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp);
	void ackPacket(int nClientSocketFD, int nCommand, const void * pData);

	/**  Receive CMP Request **/
	int cmpUnknow(int nSocket, int nCommand, int nSequence, const void * pData);
	int cmpRdmLogin(int nSocket, int nCommand, int nSequence, const void * pData);
	int cmpRdmLogout(int nSocket, int nCommand, int nSequence, const void * pData);
	int cmpRdmOperate(int nSocket, int nCommand, int nSequence, const void * pData);

	/** Send CMP Response **/
	int cmpResponse(const int nSocket, const int nCommandId, const int nSequence, const char * szData = 0);
	int cmpRdmLoginResponse(int nSocket, int nSequence, const char * szData);
	int cmpRdmOperateResponse(int nSocket, int nSequence, const char * szData);

	int getControllerSocketFD(std::string strControllerID);
	int cmpEnquireLinkRequest(const int nSocketFD);

private:
	CSocketServer *cmpServer;
	CCmpHandler *cmpParser;
	CSqliteHandler *sqlite;
	std::vector<int> vEnquireLink;
	CSocketClient *cmpClient;
	CRdmOperate * rdmOperate;

	typedef int (CController::*MemFn)(int, int, int, const void *);
	MemFn cmpRequest[MAX_FUNC_POINT];

};
