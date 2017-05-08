#pragma once

#include <string>
#include <map>

#include "CSocketClient.h"
#include "ICallback.h"
#include "CMPData.h"
#include "DoorAccessControl/DoorAccessHandler.h"

using namespace std;

class CCmpHandler;
class CSocketClient;

class CClientMeetingAgent: public CSocketClient
{
public:
	void onReceive(const int nSocketFD, const void *pData);
public:
	explicit CClientMeetingAgent();
	virtual ~CClientMeetingAgent();
	int startClient(string strIP, const int nPort, const int nMsqId);
	void stopClient();
	int sendCommand(int commandID, int seqNum, string bodyData);

private:
	CMPData parseCMPData(int nSocket, int nCommand, int nSequence, const void *pData, bool isBodyExist);
	
	CCmpHandler *cmpParser;
	typedef int (CClientMeetingAgent::*MemFn)(int, int, int, const void *);
	map<int, MemFn> mapFunc;

	//MeetingAgent Request for bind and unbind
	void cmpBindRequest();
	void cmpUnbindRequest();


	//MeetingAgent Response for bind and unbind
	int cmpBind(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpUnbind(int nSocket, int nCommand, int nSequence, const void *pData);


	//MeetingAgent Request for SmartBuilding
	int cmpQRCodeToken(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAPPVersion(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpGetMeetingData(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *pData);

	/**
	 * returns Unix timestamp in milliseconds
	 */
	int64_t unixTimeMilli();

	DoorAccessHandler doorAccessHandler;
};
