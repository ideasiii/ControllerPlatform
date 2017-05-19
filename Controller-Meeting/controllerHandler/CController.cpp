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

#define ENQUIRE_LINK_INTERVAL 10 // second

void *threadStartRoutine_CController_enquireLink(void *args)
{
	return 0;



	_log("[CController] threadStartRoutine_CController_enquireLink() step in");
	auto ctlr = reinterpret_cast<CController*>(args);
	ctlr->tdEnquireLinkTid = pthread_self();

	while (true)
	{
		auto& clientMeetingAgent = ctlr->agentClient;
		if (clientMeetingAgent != nullptr)
		{
			if (!clientMeetingAgent->isValidSocketFD())
			{
				_log("[CController] threadStartRoutine_CController_enquireLink() invalid fd");
			}
			else
			{
				clientMeetingAgent->request(clientMeetingAgent->getSocketfd(),
					enquire_link_request, STATUS_ROK, getSequence(), NULL);
			}
		}

		sleep(ENQUIRE_LINK_INTERVAL);
	}

	_log("[CController] threadStartRoutine_CController_enquireLink() step out");

	return NULL;
}

CController::CController() :
		mnMsqKey(-1), agentClient(nullptr),
		tdEnquireLink(nullptr)
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

	agentClient.reset(new CClientMeetingAgent(this));
	nRet = agentClient->initMember(config);
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

	tdEnquireLink.reset(new CThreadHandler());
	tdEnquireLink->createThread(threadStartRoutine_CController_enquireLink, this);

	return nRet;
}

int CController::onFinish(void* nMsqKey)
{
	if (tdEnquireLink != nullptr)
	{
		tdEnquireLink->threadCancel(tdEnquireLinkTid);
		tdEnquireLink->threadJoin(tdEnquireLinkTid);
		tdEnquireLink = nullptr;
	}

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

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE:
		_log("[CController] EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE");
		break;

	case EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT:
		_log("[CController] EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT");
		// TODO 重新連線
		// TODO startClientMeetingAgent()?????????????
		// TODO 用 enquire link 
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
	case MESSAGE_EVENT_CLIENT_MEETING_AGENT:
		_log("[CController] onHandleMessage(): MESSAGE_EVENT_CLIENT_MEETING_AGENT");
		if (message.arg[0] == -1)
		{
			_log("[CController] connection to agent is broken!@$#%^&*(*^%$#Q@#%%&^*&%$^%#");
		}
		break;
	case MESSAGE_EVENT_CLIENT_AMX_CONTROLLER:
		_log("[CController] onHandleMessage(): MESSAGE_EVENT_CLIENT_AMX_CONTROLLER");
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
