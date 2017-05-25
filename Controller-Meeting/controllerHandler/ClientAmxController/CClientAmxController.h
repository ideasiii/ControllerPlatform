#pragma once

#include <memory>
#include <string>
#include "CCmpClient.h"

class EnquireLinkYo;

class CClientAmxController : public CCmpClient
{
public:
	explicit CClientAmxController(CObject *controller, const std::string &serverIp, int userPort, int validationPort);
	virtual ~CClientAmxController();

	// Establish connection to controller, start enquire link thread,
	// start message receiving loop with message queue ID 'msqKey'.
	int startClient(int msqKey); 

	// Stop enquire link thread, stop message receiving loop,
	// disconnect from controller.
	void stopClient();

	std::string getServerIp();
	int getUserPort();
	int getValidationPort();

protected:
	// 當收到 server 的 response PDU 時
	int onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody) override;

	// AMX controller 送來驗證 token 是否有效的請求
	int onAuthenticationRequest(int nSocket, int nCommand, int nSequence, const void *szBody) override;

	std::string taskName() override;

private:
	const std::string serverIp;
	const int userPort;
	const int validationPort;

	std::unique_ptr<EnquireLinkYo> enquireLinkYo;

	// do not delete this or the whole world will collapse
	CObject * const mpController;

	bool validateToken(const std::string& reqId, const std::string& reqToken, int64_t when);
};
