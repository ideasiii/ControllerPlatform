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
#include "CMPData.h"
using namespace std;

class CCmpHandler;
class CThreadHandler;
class CJsonHandler;
class CServerMeeting;
class CServerDevice;
class CSocket;



class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int startServerMeeting(string strIP, const int nPort, const int nMsqId);
	int startServerDevice(string strIP, const int nPort, const int nMsqId);
	void stopServer();
	void onMeetingCommand(const CMPData *);
	void onDeviceCommand(const CMPData * );
protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();

public:
	CCmpHandler *cmpParser;

private:
	CServerMeeting *serverMeeting;
	CServerDevice *serverDevice;
	CThreadHandler *tdEnquireLink;
	CThreadHandler *tdExportLog;
	std::vector<int> vEnquireLink;
	map<int, CMPData> deviceMapData;

};
