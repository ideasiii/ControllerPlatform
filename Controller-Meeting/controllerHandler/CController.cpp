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

CController::CController() :
		mnMsqKey(-1), agentClient(nullptr)
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

	_log("[CController] onCreated() mnMsqKey = %d", mnMsqKey);
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	// We use the config lies under the same directory with process image
	//string strConfPath = reinterpret_cast<const char*>(szConfPath);
	
	std::string strConfPath = HiddenUtility::getConfigPathInProcessImageDirectory();
	_log("[CController] onInitial() Config path = `%s`", strConfPath.c_str());
	
	if(strConfPath.empty())
	{
		return FALSE;
	}
	
	std::unique_ptr<CConfig> config = make_unique<CConfig>();
	int nRet = config->loadConfig(strConfPath);
	if(!nRet)
	{
		_log("[CController] onInitial() config->loadConfig() failed");
		return FALSE;
	}

	nRet = initAgentClient(config);
	if (nRet == FALSE)
	{
		_log("[CController] onInitial() agentClient->configMember() failed");
		return FALSE;
	}

	auto clientAmxControllerRet = CClientAmxControllerFactory::createFromConfig(config);
	if (clientAmxControllerRet == nullptr)
	{
		_log("[CController] onInitial(): CClientAmxController cannot be instantiated");
		return FALSE;
	}
	amxControllerClient.reset(clientAmxControllerRet);

	nRet = agentClient->startClient(mnMsqKey);
	if (nRet < 0)
	{
		return FALSE;
	}

	//amxControllerClient->start();

	return nRet;
}

int CController::onFinish(void* nMsqKey)
{
	if (amxControllerClient != nullptr)
	{
		// amxControllerClient->stopClient();
		amxControllerClient = nullptr;
	}

	if (agentClient != nullptr)
	{
		agentClient->stopClient();
		agentClient = nullptr;
	}

	return TRUE;
}

int CController::initAgentClient()
{
	std::string strConfPath = HiddenUtility::getConfigPathInProcessImageDirectory();
	std::unique_ptr<CConfig> config = make_unique<CConfig>();
	int nRet = config->loadConfig(strConfPath);
	if (!nRet)
	{
		_log("[CController] initAgentClient() config->loadConfig() failed");
		return FALSE;
	}

	return initAgentClient(config);
}

int CController::initAgentClient(std::unique_ptr<CConfig>& config)
{
	if (agentClient != nullptr)
	{
		agentClient.release();
	}

	agentClient.reset(new CClientMeetingAgent(this));
	return agentClient->initMember(config);
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE:
		_log("[CController] EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE");
		break;
	case EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT:
		_log("[CController] EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT");
		_log("[CController] connection to agent is broken!@$#%^&*(*^%$#Q@#%%dcfvgbjhujniklhguytr5e4aw3&^*&%$^%#");
		agentClient->stopClient();

		sleep(3);
		_log("[CController] Reconnecting to agent");
		int nRet = initAgentClient();
		if (nRet == TRUE)
		{
			_log("[CController] Reconnecting to agent OK");
			agentClient->startClient(mnMsqKey);
		}
		else
		{
			_log("[CController] Reconnecting to agent FAILED");
		}

		// TODO 重新連線
		break;
	default:
		_log("[CController] Unknown message command %s", numberToHex(nCommand).c_str());
		break;
	}
}

void CController::onHandleMessage(Message &message)
{
	int nRet;

	switch (message.what)
	{
	case MESSAGE_WHAT_CLIENT_MEETING_AGENT:
		_log("[CController] onHandleMessage(): MESSAGE_WHAT_CLIENT_MEETING_AGENT");
		if (message.arg[0] == -1)
		{
			_log("[CController] connection to agent is broken!@$#%^&*(*^%$#Q@#%%dcfvgbjhujniklhguytr5e4aw3&^*&%$^%#");
		}
		break;
	case MESSAGE_WHAT_CLIENT_AMX_CONTROLLER:
		_log("[CController] onHandleMessage(): MESSAGE_WHAT_CLIENT_AMX_CONTROLLER");
		break;
	default:
		_log("[CController] onHandleMessage(): Message will not be processed");
		break;
	}
}

std::string CController::taskName()
{
	return "CController";
}
