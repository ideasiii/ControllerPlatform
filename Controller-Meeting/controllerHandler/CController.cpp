#include "CController.h"

#include <list>
#include <limits.h>
#include <random>
#include <sys/prctl.h>
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
#define RECONNECT_INTERVAL 8

void *threadStartRoutine_CController_reconnectToAgent(void *argv);
void *threadStartRoutine_CController_reconnectToAmx(void *argv);

CController::CController() :
	mnMsqKey(-1), 
	agentClient(nullptr), amxControllerClient(nullptr),
	agentReconnectThreadId(0), amxReconnectThreadId(0)
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

	if (agentReconnectThreadId > 0)
	{
		threadCancel(agentReconnectThreadId);
		agentReconnectThreadId = 0;
	}
	
	if (amxReconnectThreadId > 0)
	{
		threadCancel(amxReconnectThreadId);
		amxReconnectThreadId = 0;
	}

	return TRUE;
}

// 初始化 CClientMeetingAgent。
// 在斷線後要重新連線的時候也會重新初始化一個 CClientMeetingAgent
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
		//_log(LOG_TAG" EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT");
		_log(LOG_TAG_COLORED" Connection to agent is broken!@$#%^&*(*^%$#Q@#%%dcfvgtr5e3&^*&%$^%#");
		
		agentClient->stopClient();
		sleep(3);
		
		if (agentReconnectThreadId < 1)
		{
			_log(LOG_TAG" Reconnecting to agent");
			createThread(threadStartRoutine_CController_reconnectToAgent, this, "agentReconnect");
		}
				
		break;
	case EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_AMX:
		//_log(LOG_TAG" EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_AMX");
		_log(LOG_TAG_COLORED" Connection to AMX controller is broken! JKSKALXUH@OUW)233333333333");

		amxControllerClient->stopClient();
		sleep(3);

		if (amxReconnectThreadId < 1)
		{
			_log(LOG_TAG" Reconnecting to AMX");
			this->createThread(threadStartRoutine_CController_reconnectToAmx, this, "amxReconnect");
		}

		break;
	default:
		_log(LOG_TAG" Unknown message command %s", numberToHex(nCommand).c_str());
		break;
	}
}

void CController::onHandleMessage(Message &message)
{
	switch (message.what)
	{
	case MESSAGE_WHAT_CLIENT_MEETING_AGENT:
		_log(LOG_TAG" onHandleMessage(): MESSAGE_WHAT_CLIENT_MEETING_AGENT");
		break;
	case MESSAGE_WHAT_CLIENT_AMX_CONTROLLER:
		_log(LOG_TAG" onHandleMessage(): MESSAGE_WHAT_CLIENT_AMX_CONTROLLER");
		break;
	default:
		_log(LOG_TAG" onHandleMessage(): unknown message.what %d", message.what);
		break;
	}
}

std::string CController::taskName()
{
	return "CController";
}

void *threadStartRoutine_CController_reconnectToAgent(void *argv)
{
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, (unsigned long)"agentReconnect");
	_log(LOG_TAG"threadStartRoutine_CController_reconnectToAgent() step in");
	
	auto ctlr = reinterpret_cast<CController*>(argv); 
	ctlr->agentReconnectThreadId = pthread_self();
	
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<> dist(RECONNECT_INTERVAL/2, RECONNECT_INTERVAL*2);

	while (true)
	{
		auto client = ctlr->amxControllerClient.get();
		if (client == nullptr
			|| client->startClient(ctlr->mnMsqKey) == TRUE)
		{
			break;
		}

		int reconnectInterval = (int)dist(mt);
		_log("[CController] agentReconnect waits %ds for next attempt", reconnectInterval);
		sleep(reconnectInterval);
	}

	ctlr->agentReconnectThreadId = 0;
	_log(LOG_TAG" %s threadStartRoutine_CController_reconnectToAgent() step out");
	return 0;
}

void *threadStartRoutine_CController_reconnectToAmx(void *argv)
{
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, (unsigned long)"amxReconnect");
	_log(LOG_TAG"threadStartRoutine_CController_reconnectToAmx() step in");

	auto ctlr = reinterpret_cast<CController*>(argv);
	ctlr->amxReconnectThreadId = pthread_self();
	
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<> dist(RECONNECT_INTERVAL/2, RECONNECT_INTERVAL*2);

	while (true)
	{
		auto client = ctlr->amxControllerClient.get();
		if (client == nullptr
			|| client->startClient(ctlr->mnMsqKey) == TRUE)
		{
			break;
		}

		int reconnectInterval = (int)dist(mt);
		_log("[CController] amxReconnect waits %ds for next attempt", reconnectInterval);
		sleep(reconnectInterval);
	}

	ctlr->amxReconnectThreadId = 0; 
	_log(LOG_TAG" %s threadStartRoutine_CController_reconnectToAmx() step out");
	return 0;
}
