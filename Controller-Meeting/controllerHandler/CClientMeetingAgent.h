#pragma once

#include <string>
#include <map>
#include <memory>

#include "CMPData.h"
#include "CCmpClient.h"
#include "DoorAccessControl/DoorAccessHandler.h"
#include "ICallback.h"

class AmxControllerInfo;
class AppVersionHandler; 
class CCmpHandler;
class CConfig;
class CSocketClient;
class EnquireLinkYo;

class CClientMeetingAgent: public CCmpClient
{
public:
	explicit CClientMeetingAgent(CObject *controller);
	virtual ~CClientMeetingAgent();

	// Intializes members that needs parameters in config.
	// Returns FALSE if anything bad happens
	int initMember(std::unique_ptr<CConfig> &config);

	int startClient(int msqKey); 
	void stopClient();

protected:
	int onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody) override;
	
	//MeetingAgent Request for SmartBuilding
	int onSmartBuildingQrCodeToken(int nSocket, int nCommand, int nSequence, const void *szBody) override;
	int onSmartBuildingAppVersion(int nSocket, int nCommand, int nSequence, const void *szBody) override;
	int onSmartBuildingMeetingData(int nSocket, int nCommand, int nSequence, const void *szBody) override;
	int onSmartBuildingAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *szBody) override;

private:
	int initMeetingAgentServerParams(std::unique_ptr<CConfig> &config);

	std::string agentIp;
	int agentPort;

	DoorAccessHandler doorAccessHandler;
	std::unique_ptr<AppVersionHandler> appVersionHandler;
	std::unique_ptr<AmxControllerInfo> amxControllerInfo;
	std::unique_ptr<EnquireLinkYo> enquireLinkYo;
	CObject *mpController;
};
