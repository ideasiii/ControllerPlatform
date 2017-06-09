#include "CClientAmxController.h"

#include <regex>
#include <sstream>
#include "packet.h"
#include "event.h"
#include "utility.h"
#include "../../enquireLinkYo/EnquireLinkYo.h"
#include "../HiddenUtility.hpp"
#include "../TestStringsDefinition.h"
#include "CMysqlHandler.h"
#include "JSONObject.h"
#include "LogHandler.h"

#define TASK_NAME "ClientAMX"
#define LOG_TAG "[CClientAmxController]"
#define LOG_TAG_COLORED "[\033[1;31mCClientAmxController\033[0m]"

// UUID pattern, which does not conform to strict standards
#define UUID_PATTERN R"([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})"


CClientAmxController::CClientAmxController(CObject *controller, const std::string &serverIp,
	int userPort, int validationPort, MysqlSourceInfo& mysqlSrc) :
	serverIp(serverIp), userPort(userPort), validationPort(validationPort), 
	mysqlSourceInfo(mysqlSrc), mpController(controller)
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
		_log(LOG_TAG" onAuthenticationRequest() Miss something in JSON");
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
	std::regex uuidRegex(UUID_PATTERN);

	if (!regex_match(reqId, uuidRegex))
	{
		_log(LOG_TAG" validateToken() ID %s is not a valid UUID", reqId.c_str());
		return false;
	}
	else if (!regex_match(reqToken, uuidRegex))
	{
		_log(LOG_TAG" validateToken() Token %s is not a valid UUID", reqToken.c_str());
		return false;
	}

	if (tokenCache.find(reqToken) != tokenCache.end())
	{
		_log(LOG_TAG" validateToken() Token cache hit (%s)", reqToken.c_str());
		auto& hitRecord = tokenCache[reqToken];
		return hitRecord.userUuid.compare(reqId) == 0
			&& hitRecord.validFrom <= when
			&& hitRecord.goodThrough >= when;
	}

	CMysqlHandler mysql;
	int nRet = mysql.connect(mysqlSourceInfo.host, mysqlSourceInfo.database, mysqlSourceInfo.user,
		mysqlSourceInfo.password);

	if (FALSE == nRet)
	{
		_log(LOG_TAG" validateToken() Mysql Error: %s", mysql.getLastError().c_str());
		return true;
	}

	list<map<string, string> > listRet;
	string strSQL = "SELECT t.time_start, t.time_end FROM meeting.amx_control_token as t, meeting.user as u "
		"WHERE u.uuid = '" + reqId + "' AND t.user_id = u.id AND t.token = '" + reqToken + "' AND t.valid = 1 AND u.valid = 1;";
	nRet = mysql.query(strSQL, listRet);
	string strError = mysql.getLastError();
	mysql.close();

	if (FALSE == nRet)
	{
		_log(LOG_TAG" validateToken() Mysql Error: %s", strError.c_str());
		return true;
	}
	else if (listRet.size() < 1)
	{
		_log(LOG_TAG" validateToken() db no match token");
		return false;
	}
	else if (listRet.size() > 1)
	{
		_log(LOG_TAG" validateToken() db returned more than 1 result?");
		return true;
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
	//_DBG(LOG_TAG" onServerDisconnect() step in");
	CCmpClient::onServerDisconnect(nSocketFD);

	// let CController decide what to do
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
