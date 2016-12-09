/*
 * CControlCenter.h
 *
 *  Created on: 2015年10月20日
 *      Author: Louis Ju
 */

#pragma once

#include <string>
#include <vector>
#include <list>
#include "CObject.h"
#include "common.h"

#define MAX_FUNC_POINT		256

class CSocketServer;
class CCmpHandler;
class CSqliteHandler;
class CThreadHandler;
class CJsonHandler;
class CAccessLog;
class CAuthentication;

class CControlCenter: public CObject
{
	public:
		virtual ~CControlCenter();
		static CControlCenter* getInstance();
		int startServer(const int nPort);
		void stopServer();
		void runEnquireLinkRequest();BOOL startSqlite(const int nDBId, const std::string strDB);BOOL startMongo(
				const std::string strIP, const int nPort);

	protected:
		void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

	private:
		explicit CControlCenter();
		void onCMP(int nClientFD, int nDataLen, const void *pData);
		int sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp);
		void ackPacket(int nClientSocketFD, int nCommand, const void * pData);

		/**  Receive CMP Request **/
		int cmpUnknow(int nSocket, int nCommand, int nSequence, const void * pData);
		int cmpBind(int nSocket, int nCommand, int nSequence, const void * pData);
		int cmpUnbind(int nSocket, int nCommand, int nSequence, const void * pData);
		int cmpPowerPort(int nSocket, int nCommand, int nSequence, const void *pData);
		int cmpPowerPortState(int nSocket, int nCommand, int nSequence, const void *pData);
		int cmpAccessLog(int nSocket, int nCommand, int nSequence, const void *pData);
		int cmpInitial(int nSocket, int nCommand, int nSequence, const void *pData);
		int cmpSignup(int nSocket, int nCommand, int nSequence, const void *pData);
		int cmpSdkTracker(int nSocket, int nCommand, int nSequence, const void *pData);
		int cmpAuthentication(int nSocket, int nCommand, int nSequence, const void *pData);

		/** Send CMP Request **/
		int cmpPowerPortRequest(int nSocket, std::string strWire, std::string strPort, std::string strState);
		int cmpPowerPortStateRequest(int nSocket, std::string strWire);

		/** Send CMP Response **/
		int cmpResponse(const int nSocket, const int nCommandId, const int nSequence, const char * szData = 0);
		int cmpPowerPortStateResponse(int nSocket, int nSequence, const char * szData);
		int cmpInitialResponse(int nSocket, int nSequence, const char * szData);
		int cmpMdmLoginResponse(int nSocket, int nSequence, const char * szData);

		int getControllerSocketFD(std::string strControllerID);
		int getBindSocket(std::list<int> &listValue);
		int cmpEnquireLinkRequest(const int nSocketFD);

	private:
		CSocketServer *cmpServer;
		CCmpHandler *cmpParser;
		CSqliteHandler *sqlite;
		CThreadHandler *tdEnquireLink;
		CThreadHandler *tdExportLog;
		CAccessLog *accessLog;
		std::vector<int> vEnquireLink;
		CAuthentication* authentication;

		typedef int (CControlCenter::*MemFn)(int, int, int, const void *);
		MemFn cmpRequest[MAX_FUNC_POINT];

};