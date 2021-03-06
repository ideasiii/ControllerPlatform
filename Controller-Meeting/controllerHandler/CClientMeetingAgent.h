#pragma once

#include <string>
#include <memory>
#include "AesCrypto.h"

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

// controller-meetingagent 的 client，接收 agent 轉送的、來自 user app 的請求
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

	// 解碼 QR code 並處理相應要求
	// 回傳 JSON 字串
	std::string handleQrCodeToken(const std::string& reqUserId, const std::string& qrcodeToken);

	// 取得 user 的會議資訊
	// 回傳 JSON 字串
	std::string getMeetingsInfo(const std::string& userId);

	// 進行數位簽到
	// 返回內容為要回應給 client 的 json 字串
	std::string doDigitalSignup(const std::string& userId);

	// 取得 AMX 裝置控制 token。若 userId 在現在的時間有任何可控制 roomId 的 token，傳回之。
	// 返回內容為要回應給 client 的 json 字串
	std::string getAMXControlToken(const std::string& userId, const std::string& roomId);

	// 解碼 QR code 掃出來的字串
	// 若是預先定義的 QR code，直接回傳對應的結果，否則委派 decryptQRCodeString() 進行解密
	std::unique_ptr<JSONObject> decodeQRCodeString(const std::string& src) const;

	// 解密 QR code 掃出來的字串
	std::unique_ptr<JSONObject> decryptQRCodeString(const std::string& src) const;

	// parent of CClientMeetingAgent instance
	// do not delete this or the whole world will collapse
	CObject * const mpController;

	std::string agentIp;
	int agentPort;

	// 做過 SHA256 hash 的 QR code 解密金鑰
	uint8_t qrCodeKey[AesCrypto::KeyLength];

	DoorAccessHandler doorAccessHandler;
	std::unique_ptr<AppVersionHandler> appVersionHandler;
	std::unique_ptr<AmxControllerInfo> amxControllerInfo;
	std::unique_ptr<EnquireLinkYo> enquireLinkYo;
};
