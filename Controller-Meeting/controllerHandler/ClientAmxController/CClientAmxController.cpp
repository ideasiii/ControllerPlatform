#include "CClientAmxController.h"

#include <sstream>
#include "packet.h"
#include "event.h"
#include "utility.h"
#include "../../enquireLinkYo/EnquireLinkYo.h"
#include "../HiddenUtility.hpp"
#include "../TestStringsDefinition.h"
#include "JSONObject.h"
#include "LogHandler.h"

#define TASK_NAME "ClientAMX"
#define LOG_TAG "[CClientAmxController]"
#define LOG_TAG_COLORED "[\033[1;31mCClientAmxController\033[0m]"

CClientAmxController::CClientAmxController(CObject *controller, const std::string &serverIp, int userPort, int validationPort) :
	serverIp(serverIp), userPort(userPort), validationPort(validationPort), mpController(controller)
{
	enquireLinkYo.reset(new EnquireLinkYo("ClientAMX.ely", this,
		EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_AMX, mpController));
}

CClientAmxController::~CClientAmxController()
{
	stopClient();
}

int CClientAmxController::startClient(int msqKey)
{
	_log(LOG_TAG" Connecting to AMX controller validation service %s:%d",
		serverIp.c_str(), validationPort);

	int nRet = connectWithCallback(serverIp.c_str(), validationPort, msqKey,
		[](CATcpClient *caller, pthread_t msgRecvTid, pthread_t pktRecvTid) -> void
	{
		CClientAmxController *self = dynamic_cast<CClientAmxController *>(caller);
		if (self == nullptr)
		{
			_log(LOG_TAG" startClient() cast failed on callback");
			return;
		}

		//_log(LOG_TAG" startClient() Set receivers thread name");

		std::string msgThreadName = "AmxMsgRecv";
		std::string pktThreadName = "AmxPktRecv";

		pthread_setname_np(msgRecvTid, msgThreadName.c_str());
		pthread_setname_np(pktRecvTid, pktThreadName.c_str());

	});

	if (nRet < 0)
	{
		_log(LOG_TAG" startClient() Connecting to AMX controller FAILED");
		return FALSE;
	}

	nRet = request(getSocketfd(), bind_request, STATUS_ROK, getSequence(), NULL);
	if (nRet < 0)
	{
		_log(LOG_TAG" startClient() Binding to AMX controller FAILED");
		return FALSE;
	}

	// enquireLinkYo starts in onResponse(), when binding response is received

	return TRUE;
}

void CClientAmxController::stopClient()
{
	//_DBG(LOG_TAG" stopClient() step in"); 

	if (enquireLinkYo != nullptr)
	{
		enquireLinkYo->stop();
	}

	if (!isValidSocketFD())
	{
		_log(LOG_TAG" stopClient() socket fd is not valid, quit stopping");
		return;
	}

	request(getSocketfd(), unbind_request, STATUS_ROK, getSequence(), NULL);
	stop();

	//_DBG(LOG_TAG" stopClient() step out");
}

int CClientAmxController::onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody)
{
	//_DBG(LOG_TAG" onResponse() step in");

	switch ((unsigned int)nCommand)
	{
	case enquire_link_response:
		_log(LOG_TAG_COLORED" onResponse() enquire_link_response");
		enquireLinkYo->zeroBalance();
		break;
	case bind_response:
		_log(LOG_TAG_COLORED" onResponse() bind_response; bind ok, start EnquireLinkYo");
		enquireLinkYo->start();
		break;
	case unbind_response:
		_log(LOG_TAG_COLORED" onResponse() unbind_response");
		break;
	default:
		_log(LOG_TAG" onResponse() unhandled nCommand %s", numberToHex(nCommand).c_str());
		break;
	}
	
	return TRUE;
}

int CClientAmxController::onAuthenticationRequest(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	JSONObject reqJson(reinterpret_cast<const char *>(szBody));
	if (!reqJson.isValid())
	{
		_log(LOG_TAG" onAuthenticationRequest() This is not JSON");
		response(getSocketfd(), nCommand, STATUS_RINVBODY, nSequence, NULL);
		return TRUE;
	}

	std::string reqToken = reqJson.getString("TOKEN", "");
	std::string reqId = reqJson.getString("ID", "");
	reqJson.release();

	if (reqToken.empty() || reqId.empty())
	{
		_log(LOG_TAG" onAuthenticationRequest() Missed something in JSON");
		response(getSocketfd(), nCommand, STATUS_RINVJSON, nSequence, NULL);
		return TRUE;
	}

	int64_t now = HiddenUtility::unixTimeMilli();
	bool authOk = validateToken(reqId, reqToken, now);

	std::stringstream ss;
	ss << "{\"ID\": \"" << reqId << "\", \"AUTH\": \"";

	if (authOk)
	{
		_log(LOG_TAG_COLORED" onAuthenticationRequest() `%s` w/ token `%s` PASSED on validation",
			reqId.c_str(), reqToken.c_str());
		ss << "y";
	}
	else
	{
		_log(LOG_TAG_COLORED" onAuthenticationRequest() `%s` w/ token `%s` FAILED on validation",
			reqId.c_str(), reqToken.c_str());
		ss << "n";
	}
	
	ss << "\"}";
	
	std::string respBody = ss.str();
	response(getSocketfd(), nCommand, STATUS_ROK, nSequence, respBody.c_str());

	return TRUE;
}

// Check given 'reqToken' sent by 'reqId' is valid at 'when'.
// 'when' is the point we received the validation request, in unix epoch (milliseconds, UTC)
bool CClientAmxController::validateToken(const std::string& reqId, const std::string& reqToken, const int64_t when)
{
	// check retrieved result is not empty
	if (reqId.compare(TEST_USER_HAS_MEETING_IN_001) != 0 || reqToken.compare(TEST_AMX_TOKEN) != 0)
	{
		return false;
	}

	int64_t tokenValidFrom = when - (10 * 1000);
	int64_t tokanGoodThrough = when + (10 * 1000);

	// check token is valid at time given
	return when >= tokenValidFrom && when <= tokanGoodThrough;
}

void CClientAmxController::onServerDisconnect(unsigned long int nSocketFD)
{
	//_DBG(LOG_TAG" onServerDisconnect() step in");
	CCmpClient::onServerDisconnect(nSocketFD);

	// let CController decide what to do when disconnected
	_log(LOG_TAG" Server actively disconnected");
	mpController->sendMessage(EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_AMX,
		0, 0, NULL);

	//_DBG(LOG_TAG" onServerDisconnect() step out");
}

std::string CClientAmxController::getServerIp()
{
	return serverIp;
}
	
int CClientAmxController::getUserPort()
{
	return userPort;
}

int CClientAmxController::getValidationPort()
{
	return validationPort;
}

std::string CClientAmxController::taskName()
{
	return TASK_NAME;
}
