/*
 * CCmpServer.h
 *
 *  Created on: 2017年3月16日
 *      Author: Jugo
 */

#pragma once

#include <map>
#include "CATcpServer.h"

class CCmpServer: public CATcpServer
{
	typedef struct _CONF_CMP_SERVER
	{
		bool bUseQueueReceive;
		void init()
		{
			bUseQueueReceive = false;
		}
	} CONF_CMP_SERVER;

public:
	CCmpServer();
	virtual ~CCmpServer();
	int request(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData);
	int response(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData);
	void idleTimeout(bool bRun, int nIdleTime);
	void setUseQueueReceive(bool bEnable);

protected:
	void onTimer(int nId);
	void onReceive(unsigned long int nSocketFD, int nDataLen, const void* pData);
	int onTcpReceive(unsigned long int nSocketFD);

	/**
	 * Controller Message Protocol (CMP) Request Callback.
	 * Parameters:
	 * 				nSocket: Client Socket File Description
	 * 				nCommand: Command ID
	 * 				nSequence: CMP Packet Sequence
	 * 				szBody: CMP Body Data
	 */
protected:
	virtual int onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onInitial(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onSignin(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onAccesslog(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onSemanticWord(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;

	//for other Controller Data
	virtual int onFCMIdRegister(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onFBToken(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onWirelessPowerCharge(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;

	//for Controller-Meeting Data
	virtual int onQRCodeToken(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onAPPVersion(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onGetMeetingData(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onEnquireLink(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;

private:
	typedef int (CCmpServer::*MemFn)(int, int, int, const void *);
	std::map<int, MemFn> mapFunc;
	int sendPacket(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData);
	CONF_CMP_SERVER *confCmpServer;

};
