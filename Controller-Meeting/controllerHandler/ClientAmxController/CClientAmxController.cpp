#include "CClientAmxController.h"

#include <sstream>
#include "packet.h"
#include "event.h"
#include "utility.h"
#include "../../enquireLinkYo/EnquireLinkYo.h"
#include "JSONObject.h"
#include "LogHandler.h"

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
}

int CClientAmxController::startClient(int msqKey)
{
	_log(LOG_TAG" Connecting to AMX controller validation service %s:%d",
		serverIp.c_str(), validationPort);

	int nRet = connect(serverIp.c_str(), validationPort, msqKey);
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

	int nRet = request(getSocketfd(), unbind_request, STATUS_ROK, getSequence(), NULL);
	if (nRet < 0)
	{
		_log(LOG_TAG" stopClient() Unbinding from AMX Controller FAILED.");
	}
	else
	{
		_log(LOG_TAG" stopClient() Unbinding from AMX Controller OK.");
	}

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

	bool passedValidation = true;

	std::stringstream ss;
	ss << "{\"ID\": \"" << reqId << "\", \"AUTH\": \"";

	if (passedValidation)
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
	return "ClientAMX";
}
