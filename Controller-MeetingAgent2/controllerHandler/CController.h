/*
 * CController.h
 *
 *  Created on: 2017年4月8日
 *      Author: Joe
 */

#pragma once

#include <string>
#include <map>


#include "CApplication.h"
#include "CMPData.h"


class CMPData;
class CServerMeeting;
class CServerDevice;

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();
	void onDeviceCommand(const CMPData * );
	void onMeetingCommand(const CMPData * );
	int startServerMeeting(std::string strIP, const int nPort, const int nMsqId);
	int startServerDevice(std::string strIP, const int nPort, const int nMsqId);
protected:
	int onCreated(void* nMsqKey);
	/**
	 *  Main Process run will callback onInitial
	 */
	int onInitial(void* szConfPath);

	/*
	 *  Main Process terminator will callback onFinish
	 */
	int onFinish(void* nMsqKey);

	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
private:

private:
	CServerMeeting *serverMeeting;
	CServerDevice *serverDevice;
	map<int, CMPData> deviceMapData;
	int mnMsqKey;
};
