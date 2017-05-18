#include "CController.h"

#include <list>
#include <limits.h>
#include "common.h"
#include "event.h"
#include "packet.h"
#include "utility.h"
#include "CClientMeetingAgent.h"
#include "CConfig.h"
#include "CThreadHandler.h"
#include "HiddenUtility.hpp"
#include "JSONObject.h"
#include "AppVersionHandler/AppVersionHandler.h"

using namespace std;

#define ENQUIRE_LINK_INTERVAL 10 // second

void *threadStartRoutine_CController_enquireLink(void *args)
{
	_log("[CController] threadStartRoutine_CController_enquireLink() step in");
	auto ctlr = reinterpret_cast<CController*>(args);
	ctlr->tdEnquireLinkTid = pthread_self();

	while (true)
	{
		auto& clientMeetingAgent = ctlr->clientAgent;
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
		mnMsqKey(-1), clientAgent(nullptr),
		tdEnquireLink(nullptr), tdExportLog(nullptr)
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

	clientAgent.reset(new CClientMeetingAgent(this));
	nRet = clientAgent->initMember(config);
	if (nRet == FALSE)
	{
		_log("[CController] onInitial() clientAgent->configMember() failed");
		return FALSE;
	}

move client AMX controller to CController

	_log("[CController] Connecting to MeetingAgent %s:%d", serverMeetingAgentIp.c_str(), serverMeetingAgentPort);
	nRet = clientAgent->startClient(mnMsqKey);
	if (nRet < 0)
	{
		return FALSE;
	}

	tdEnquireLink.reset(new CThreadHandler());
	tdExportLog.reset(new CThreadHandler());

	tdEnquireLink->createThread(threadStartRoutine_CController_enquireLink, this);

	return nRet;
}

int CController::onFinish(void* nMsqKey)
{
	if (clientAgent != nullptr)
	{
		clientAgent->stopClient();
		clientAgent = nullptr;
	}

	if (tdEnquireLink != nullptr)
	{
		tdEnquireLink->threadCancel(tdEnquireLinkTid);
		tdEnquireLink->threadJoin(tdEnquireLinkTid);
		tdEnquireLink = nullptr;
	}

	return TRUE;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE:
		_log("[CController] EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE");
		_log("[CController] HOW TO RECEIVE????");
		//clientAgent->onReceive(nId, pData);
		break;

	case EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT:
		_log("[CController] EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT");
		// TODO 重新連線
		// TODO startClientMeetingAgent()?????????????
		// TODO 用 enquire link 
		break;

	default:
		_log("[CController] Unknown message command: %s", numberToHex(nCommand).c_str());
		break;
	}
}

void CController::onHandleMessage(Message &message)
{
	_log("[CController] onHandleMessage(): Message will not be processed");
}

std::string CController::taskName()
{
	return "CController";
}
