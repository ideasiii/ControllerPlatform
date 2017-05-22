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
	// Instantiate this class, and use controller to send message to queue
	explicit CClientMeetingAgent(CObject *controller);
	virtual ~CClientMeetingAgent();

	// Intializes members that needs parameters in config.
	// Returns FALSE if anything bad happens
	int initMember(std::unique_ptr<CConfig> &config);

	// Establish connection to agent, start enquire link thread,
	// start message receiving loop with message queue ID 'msqKey'.
	int startClient(int msqKey); 

	// Stop enquire link thread, stop message receiving loop,
	// disconnect from agent.
	void stopClient();

protected:
	// 當收到 server 的 response PDU 時
	int onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody) override;
	
	// 使用者掃了 QR code，看可以做什麼
	int onSmartBuildingQrCodeToken(int nSocket, int nCommand, int nSequence, const void *szBody) override;

	// 使用者要拿最新的 app 版本資訊
	int onSmartBuildingAppVersion(int nSocket, int nCommand, int nSequence, const void *szBody) override;

	// 使用者要拿自己的會議資訊
	int onSmartBuildingMeetingData(int nSocket, int nCommand, int nSequence, const void *szBody) override;

	// 使用者要拿 AMX 裝置控制的 token
	int onSmartBuildingAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *szBody) override;
	
	void onServerDisconnect(unsigned long int nSocketFD) override;

private:
	// 初始化連線到 agent 需要的參數
	int initMeetingAgentServerParams(std::unique_ptr<CConfig> &config);

	std::string agentIp;
	int agentPort;

	DoorAccessHandler doorAccessHandler;
	std::unique_ptr<AppVersionHandler> appVersionHandler;
	std::unique_ptr<AmxControllerInfo> amxControllerInfo;
	std::unique_ptr<EnquireLinkYo> enquireLinkYo;

	CObject *mpController;
};
