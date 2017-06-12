#pragma once

#include <atomic>
#include <string>
#include <map>
#include <memory>

#include "CCmpClient.h"
#include "DoorAccessControl/DoorAccessHandler.h"
#include "ICallback.h"

class AmxControllerInfo;
class AppVersionHandler; 
class CCmpHandler;
class CConfig;
class CSocketClient;
class EnquireLinkYo;
class JSONObject;

class CClientMeetingAgent : public CCmpClient
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
	int onSmartBuildingQrCodeTokenRequest(int nSocket, int nCommand, int nSequence, const void *szBody) override;

	// 使用者要拿最新的 app 版本資訊
	int onSmartBuildingAppVersionRequest(int nSocket, int nCommand, int nSequence, const void *szBody) override;

	// 使用者要拿自己的會議資訊
	int onSmartBuildingMeetingDataRequest(int nSocket, int nCommand, int nSequence, const void *szBody) override;

	// 使用者要拿 AMX 裝置控制的 token
	int onSmartBuildingAMXControlAccessRequest(int nSocket, int nCommand, int nSequence, const void *szBody) override;
	
	void onServerDisconnect(unsigned long int nSocketFD) override;
	std::string taskName() override;

private:
	// 初始化連線到 agent 需要的參數
	int initMeetingAgentServerParams(std::unique_ptr<CConfig>& config);

	// 解碼 QR code 掃出來的字串
	// 不再使用時必須 delete 該物件
	JSONObject *decodeQRCodeString(std::string& src);

	std::string agentIp;
	int agentPort;

	DoorAccessHandler doorAccessHandler;
	std::unique_ptr<AppVersionHandler> appVersionHandler;
	std::unique_ptr<AmxControllerInfo> amxControllerInfo;
	std::unique_ptr<EnquireLinkYo> enquireLinkYo;

	// do not delete this or the whole world will collapse
	CObject * const mpController;
};
