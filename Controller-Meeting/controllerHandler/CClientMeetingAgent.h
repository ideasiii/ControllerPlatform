#pragma once

#include <string>
#include <map>
#include <memory>

#include "CSocketClient.h"
#include "ICallback.h"
#include "CMPData.h"
#include "DoorAccessControl/DoorAccessHandler.h"

class CClientAmxController;
class CCmpHandler;
class CConfig;
class CSocketClient;
class UserAppVersionHandler;

class CClientMeetingAgent: public CSocketClient
{
public:
	void onReceive(const int nSocketFD, const void *pData);
public:
	// Ownership of appLinkHandler will be transfered!
	// Do not use appVerHandler outside CClientMeetingAgent later on
	explicit CClientMeetingAgent();
	virtual ~CClientMeetingAgent();

	// Intializes members that needs parameters in config.
	// Returns FALSE if anything bad happens
	int initMember(std::unique_ptr<CConfig> &config);
	int initUserAppVersionHandler(std::unique_ptr<CConfig> &config);

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
	int cmpBindResponse(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpUnbindResponse(int nSocket, int nCommand, int nSequence, const void *pData);


	//MeetingAgent Request for SmartBuilding
	int cmpQRCodeToken(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAPPVersion(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpGetMeetingData(int nSocket, int nCommand, int nSequence, const void *pData);
	int cmpAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *pData);
	
	DoorAccessHandler doorAccessHandler;
	unique_ptr<UserAppVersionHandler> userAppVersionHandler;
	unique_ptr<CClientAmxController> amxControllerClient;
};
