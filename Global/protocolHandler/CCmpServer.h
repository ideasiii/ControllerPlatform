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
	int request(const int nSocketFD, int nCommand, int nStatus, int nSequence, const char *szData);
	int response(const int nSocketFD, int nCommand, int nStatus, int nSequence, const char *szData);
	void idleTimeout(bool bRun, int nIdleTime);
	void setUseQueueReceive(bool bEnable);

protected:
	void onReceive(unsigned long int nSocketFD, int nDataLen, const void* pData);

	/**
	 *  Parent CATcpServer call child overload funcation.
	 *  自行定義 TCP 接收層，須實作 socket recv
	 */
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
	virtual void onTimer(int nId);
	virtual void onClientConnect(unsigned long int nSocketFD);
	virtual void onClientDisconnect(unsigned long int nSocketFD);
	virtual int onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody);
	virtual int onBind(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onUnbind(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onInitial(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;
	virtual int onUpdate(int nSocket, int nCommand, int nSequence, const void *szBody)
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

	virtual int onDie(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;

	// 虛擬運動教練
	virtual int onWheelpies(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;

	//=============== AMX Service ======================//
	virtual int onAmxControl(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		printf("[CCmpServer] onAmxControl Command: %d Body: %s\n", nCommand, reinterpret_cast<const char*>(szBody));
		return 0;
	}
	;
	virtual int onAmxStatus(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		printf("[CCmpServer] onAmxStatus Command: %d Body: %s\n", nCommand, reinterpret_cast<const char*>(szBody));
		return 0;
	}
	;

protected:
	virtual std::string taskName();
private:
	typedef int (CCmpServer::*MemFn)(int, int, int, const void *);
	std::map<int, MemFn> mapFunc;
	int sendPacket(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData);
	CONF_CMP_SERVER *confCmpServer;
	std::string strTaskName;
};
