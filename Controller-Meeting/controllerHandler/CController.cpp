#include <list>
#include <memory>
#include "event.h"
#include "utility.h"
#include "CClientMeetingAgent.h"
#include "CCmpHandler.h"
#include "CConfig.h"
#include "CController.h"
#include "CDataHandler.cpp"
#include "CSocket.h"
#include "CSocketClient.h"
#include "CSocketServer.h"
#include "CThreadHandler.h"
#include "ICallback.h"
#include "JSONObject.h"
#include "UserAppVersionHandler/UserApkPeekingAppVersionHandler.h"
#include "UserAppVersionHandler/UserAppVersionHandler.h"
#include "UserAppVersionHandler/UserConfigFileAppVersionHandler.h"

using namespace std;

#define CONF_BLOCK_MEETING_AGENT_CLIENT "CLIENT MEETING_AGENT"

CController::CController() :
		mnMsqKey(-1), mCClientMeetingAgent(nullptr),
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
	string strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial() Config path = %s", strConfPath.c_str());
	
	if(strConfPath.empty())
	{
		return FALSE;
	}
	
	tdEnquireLink = new CThreadHandler();
	tdExportLog = new CThreadHandler();
	
	std::unique_ptr<CConfig> config = make_unique<CConfig>();
	int nRet = config->loadConfig(strConfPath);
	
	if(!nRet)
	{
		_log("[CController] onInitial() config->loadConfig() failed");
		return FALSE;
	}

	string strServerIp = config->getValue(CONF_BLOCK_MEETING_AGENT_CLIENT, "server_ip");
	string strPort = config->getValue(CONF_BLOCK_MEETING_AGENT_CLIENT, "port");
	
	if (strServerIp.empty() || strPort.empty())
	{
		_log("[CController] onInitial() agent server config 404");
		return FALSE;
	}

	int nPort;
	convertFromString(nPort, strPort);

	mCClientMeetingAgent = new CClientMeetingAgent();
	nRet = mCClientMeetingAgent->initMember(config);
	if (nRet == FALSE)
	{
		_log("[CController] onInitial() mCClientMeetingAgent->configMember() failed");
		return FALSE;
	}

	nRet = startClientMeetingAgent(strServerIp, nPort, mnMsqKey);
	if (nRet == FALSE)
	{
		_log("[CController] onInitial() Start CClientMeetingAgent Failed. Port: %d, MsqKey: %d", nPort, mnMsqKey);
	}
	else
	{
		_log("[CController] onInitial() Start CClientMeetingAgent Success. Port: %d, MsqKey: %d", nPort, mnMsqKey);
	}

	return nRet;
}

int CController::onFinish(void* nMsqKey)
{
	if (mCClientMeetingAgent != nullptr)
	{
		mCClientMeetingAgent->stopClient();
		delete mCClientMeetingAgent;
		mCClientMeetingAgent = nullptr;
	}

	if (tdEnquireLink != nullptr)
	{
		// TODO how to turn off?
		//tdEnquireLink->thread?????????
		delete tdEnquireLink;
		tdEnquireLink = nullptr;
	}

	if (tdExportLog != nullptr)
	{
		// TODO how to turn off?
		//tdExportLog->thread?????????
		delete tdExportLog;
		tdExportLog = nullptr;
	}

	return TRUE;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE:
		_log("[CController] EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE");
		mCClientMeetingAgent->onReceive(nId, pData);
		break;

	case EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT:
		_log("[CController] EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT");
		// TODO 重新連線
		// TODO startClientMeetingAgent()?????????????
		// TODO 用 enquire link 
		break;

	default:
		_log("[CController] Unknown message command: %d", nCommand);
		break;
	}
}

void CController::onHandleMessage(Message &message)
{
	_log("[CController] onHandleMessage(): Message will not be processed");
}

int CController::startClientMeetingAgent(string strIP, const int nPort, const int nMsqId)
{
	_log("[CController] Connecting to Meeting-Agent server, IP: %s, port: %d", strIP.c_str(), nPort);
	return mCClientMeetingAgent->startClient(strIP, nPort, nMsqId);
}
