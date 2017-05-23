#include "CController.h"

#include <list>
#include <limits.h>
#include "common.h"
#include "event.h"
#include "iCommand.h"
#include "packet.h"
#include "utility.h"
#include "CClientMeetingAgent.h"
#include "CConfig.h"
#include "CThreadHandler.h"
#include "ClientAmxController/CClientAmxController.h"
#include "ClientAmxController/CClientAmxControllerFactory.h"
#include "HiddenUtility.hpp"
#include "JSONObject.h"
#include "AppVersionHandler/AppVersionHandler.h"

using namespace std;

#define LOG_TAG "[CController]"
#define LOG_TAG_COLORED "[\033[1;31mCControllerr\033[0m]"

CController::CController() :
		mnMsqKey(-1), agentClient(nullptr),
		amxControllerClient(nullptr)
{
	// allocate resources in onInitial() instead
}

CController::~CController()
{
	// release resources in onFinish() instead
}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_MEETING;

	// I can't give any reason to use *nMsqKey instead of a predefined ID
	//mnMsqKey = *(reinterpret_cast<int*>(nMsqKey));

	_log(LOG_TAG" onCreated() mnMsqKey = %d", mnMsqKey);
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	// We use the config lies under the same directory with process image
	//string strConfPath = reinterpret_cast<const char*>(szConfPath);
	
	std::string strConfPath = HiddenUtility::getConfigPathInProcessImageDirectory();
	_log(LOG_TAG" onInitial() Config path = `%s`", strConfPath.c_str());
	
	if(strConfPath.empty())
	{
		return FALSE;
	}
	
	std::unique_ptr<CConfig> config = make_unique<CConfig>();
	int nRet = config->loadConfig(strConfPath);
	if(!nRet)
	{
		_log(LOG_TAG" onInitial() config->loadConfig() failed");
		return FALSE;
	}

	nRet = initAgentClient(config);
	if (nRet == FALSE)
	{
		_log(LOG_TAG" onInitial() agentClient->configMember() failed");
		return FALSE;
	}

	auto clientAmxControllerRet = CClientAmxControllerFactory::createFromConfig(config, this);
	if (clientAmxControllerRet == nullptr)
	{
		_log(LOG_TAG" onInitial(): CClientAmxController cannot be instantiated");
		return FALSE;
	}
	amxControllerClient.reset(clientAmxControllerRet);

	nRet = agentClient->startClient(mnMsqKey);
	if (nRet == FALSE)
	{
		return FALSE;
	}

	nRet = amxControllerClient->startClient(mnMsqKey);

	return nRet;
}

int CController::onFinish(void* nMsqKey)
{
	if (amxControllerClient != nullptr)
	{
		amxControllerClient->stopClient();
		amxControllerClient = nullptr;
	}

	if (agentClient != nullptr)
	{
		agentClient->stopClient();
		agentClient = nullptr;
	}

	return TRUE;
}

// 初始化 CClientMeetingAgent，用在斷線後要重新連線的時候用
int CController::initAgentClient()
{
	std::string strConfPath = HiddenUtility::getConfigPathInProcessImageDirectory();
	std::unique_ptr<CConfig> config = make_unique<CConfig>();
	int nRet = config->loadConfig(strConfPath);
	if (!nRet)
	{
		_log(LOG_TAG" initAgentClient() config->loadConfig() failed");
		return FALSE;
	}

	return initAgentClient(config);
}

int CController::initAgentClient(std::unique_ptr<CConfig>& config)
{
	agentClient.reset(new CClientMeetingAgent(this));
	return agentClient->initMember(config);
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE:
		_log(LOG_TAG" EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE");
		break;
	case EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT:
		_log(LOG_TAG" EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT");
		_log(LOG_TAG" connection to agent is broken!@$#%^&*(*^%$#Q@#%%dcfvgtr5e3&^*&%$^%#");
		agentClient->stopClient();

		sleep(3);
		_log(LOG_TAG" Reconnecting to agent");
		if (initAgentClient() == TRUE)
		{
			_log(LOG_TAG" Reconnecting to agent OK");
			agentClient->startClient(mnMsqKey);
		}
		else
		{
			_log(LOG_TAG" Reconnecting to agent FAILED");
		}

		// TODO 重新連線
		break;
	case EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_AMX:
		_log(LOG_TAG" EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_AMX");
		_log(LOG_TAG" connection to AMX controller is broken! JKSKALXUH@OUW)233333333333");
		amxControllerClient->stopClient();

		// TODO agent 的重新連線 ok 之後拿過來用
		break;
	default:
		_log(LOG_TAG" Unknown message command %s", numberToHex(nCommand).c_str());
		break;
	}
}

void CController::onHandleMessage(Message &message)
{
	int nRet;

	switch (message.what)
	{
	case MESSAGE_WHAT_CLIENT_MEETING_AGENT:
		_log(LOG_TAG" onHandleMessage(): MESSAGE_WHAT_CLIENT_MEETING_AGENT");
		break;
	case MESSAGE_WHAT_CLIENT_AMX_CONTROLLER:
		_log(LOG_TAG" onHandleMessage(): MESSAGE_WHAT_CLIENT_AMX_CONTROLLER");
		break;
	default:
		_log(LOG_TAG" onHandleMessage(): Message will not be processed");
		break;
	}
}

std::string CController::taskName()
{
	return "CController";
}
