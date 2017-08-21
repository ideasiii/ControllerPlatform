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
#include "../RegexPattern.h"

#define TASK_NAME "ClientAMX"
#define LOG_TAG "[CClientAmxController]"
#define LOG_TAG_COLORED "[\033[1;31mCClientAmxController\033[0m]"

CClientAmxController::CClientAmxController(CObject *controller, const std::string &serverIp,
	int userPort, int validationPort) :
	serverIp(serverIp), userPort(userPort), validationPort(validationPort),
	mpController(controller)
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
		pthread_setname_np(msgRecvTid, "AmxMsgRecv");
		pthread_setname_np(pktRecvTid, "AmxPktRecv");
	});

	if (nRet < 0)
	{
		_log(LOG_TAG" startClient() Connecting to AMX controller FAILED");
		return FALSE;
	}

	tokenCache.clear();
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
	if (enquireLinkYo != nullptr)
	{
		enquireLinkYo->stop();
	}

	if (!isValidSocketFD())
	{
		_log(LOG_TAG" stopClient() socket fd is not valid, quit stopping");
		return;
	}

	// server don't response to unbind_request
	// receiving unbind_response while destructing CCmpClient may cause segmentation fault
	request(getSocketfd(), unbind_request, STATUS_ROK, getSequence(), NULL);
	stop();
}

int CClientAmxController::onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody)
{
	switch ((unsigned int)nCommand)
	{
	case enquire_link_response:
		_log(LOG_TAG" onResponse() enquire_link_response");
		enquireLinkYo->zeroBalance();
		break;
	case bind_response:
		_log(LOG_TAG" onResponse() bind_response; bind ok, start EnquireLinkYo");
		enquireLinkYo->start();
		break;
	case unbind_response:
		_log(LOG_TAG" onResponse() unbind_response");
		break;
	default:
		_log(LOG_TAG" onResponse() unhandled nCommand %s", numberToHex(nCommand).c_str());
		break;
	}

	return TRUE;
}

int CClientAmxController::onAuthenticationRequest(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	const char *charRequestBodyData = reinterpret_cast<const char *>(szBody);
	_log(LOG_TAG" onAuthenticationRequest() body: %s", charRequestBodyData);

	JSONObject reqJson(charRequestBodyData);
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

	std::stringstream respSs;
	respSs << "{\"ID\":\"" << reqId << "\",\"AUTH\":\"";

	if (authOk)
	{
		_log(LOG_TAG_COLORED" onAuthenticationRequest() `%s` w/ token `%s` PASSED on validation",
			reqId.c_str(), reqToken.c_str());
		respSs << "y";
	}
	else
	{
		_log(LOG_TAG_COLORED" onAuthenticationRequest() `%s` w/ token `%s` FAILED on validation",
			reqId.c_str(), reqToken.c_str());
		respSs << "n";
	}

	respSs << "\"}";

	response(getSocketfd(), nCommand, STATUS_ROK, nSequence, respSs.str().c_str());
	return TRUE;
}

// Check given 'reqToken' sent by 'reqId' is valid at 'when'.
// 'when' is the point we received the validation request, in unix epoch (milliseconds, UTC)
bool CClientAmxController::validateToken(const std::string& reqId, const std::string& reqToken, const int64_t when)
{
	if (!HiddenUtility::RegexMatch(reqId, UUID_PATTERN)
		|| !HiddenUtility::RegexMatch(reqToken, UUID_PATTERN))
	{
		_log(LOG_TAG" validateToken() ID `%s` or token `%s` is not valid",
			reqId.c_str(), reqToken.c_str());
		return false;
	}

	if (tokenCache.find(reqToken) != tokenCache.end())
	{
		_log(LOG_TAG" validateToken() Token `%s` hit cache", reqToken.c_str());
		auto& hitRecord = tokenCache[reqToken];
		return hitRecord.userUuid.compare(reqId) == 0
			&& hitRecord.validFrom <= when
			&& hitRecord.goodThrough >= when;
	}

	// cache miss
	list<map<string, string>> listRet;
	string strSQL = "SELECT t.time_start, t.time_end FROM amx_control_token as t, user as u "
		"WHERE u.uuid = '" + reqId + "' AND t.user_id = u.id AND t.token = '" + reqToken + "' AND t.valid = 1 AND u.valid = 1;";

	bool bRet = HiddenUtility::selectFromDb(LOG_TAG" validateToken()", strSQL, listRet);
	if (!bRet)
	{
		return false;
	}
	else if (listRet.size() > 1)
	{
		_log(LOG_TAG" validateToken() db returned more than 1 result?");
	}

	auto& retRow = *listRet.begin();
	auto& strValidFrom = retRow["time_start"];
	auto& strGoodThrough = retRow["time_end"];

	int64_t tokenValidFrom, tokenGoodThrough;
	convertFromString(tokenValidFrom, strValidFrom);
	convertFromString(tokenGoodThrough, strGoodThrough);

	CachedTokenInfo newRecord;
	newRecord.userUuid = reqId;
	newRecord.validFrom = tokenValidFrom;
	newRecord.goodThrough = tokenGoodThrough;

	tokenCache[reqToken] = newRecord;

	// check token is valid at time given
	return when >= tokenValidFrom && when <= tokenGoodThrough;
}

void CClientAmxController::onServerDisconnect(unsigned long int nSocketFD)
{
	CCmpClient::onServerDisconnect(nSocketFD);

	// let CController decide what to do
	_log(LOG_TAG" Server actively disconnected");
	mpController->sendMessage(EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_AMX,
		0, 0, NULL);
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
