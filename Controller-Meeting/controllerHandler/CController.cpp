#include <list>
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

using namespace std;

#define CONF_BLOCK_MEETING_AGENT_CLIENT "CLIENT MEETING_AGENT"

CController::CController() :
		mnMsqKey(-1), mCClientMeetingAgent(nullptr),
		tdEnquireLink(nullptr), tdExportLog(nullptr)
{
}

CController::~CController()
{
	// release resources in onFinish() instead
}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_MEETING;
	_log("[CController] onCreated() mnMsqKey = %d", mnMsqKey);

	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	CConfig *config;
	int nRet, nPort;
	string strConfPath, strServerIp, strPort;

	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial() Config File: %s", strConfPath.c_str());
	
	if(strConfPath.empty())
	{
		return FALSE;
	}
	
	mCClientMeetingAgent = new CClientMeetingAgent();
	tdEnquireLink = new CThreadHandler();
	tdExportLog = new CThreadHandler();
	config = new CConfig();
	nRet = FALSE;
	
	if(config->loadConfig(strConfPath))
	{
		strServerIp = config->getValue(CONF_BLOCK_MEETING_AGENT_CLIENT, "ip");
		strPort = config->getValue(CONF_BLOCK_MEETING_AGENT_CLIENT, "port");

		if (strServerIp.empty() || strPort.empty())
		{
			_log("[Controller] onInitial() controller-meetingagent server info 404");
			return FALSE;
		}

		convertFromString(nPort, strPort);
		nRet = startClientMeetingAgent(strServerIp, nPort, mnMsqKey);
		
		if (!nRet)
		{
			_log("[Controller] onInitial() Create CClientMeetingAgent Failed. Port: %d, MsqKey: %d", nPort, mnMsqKey);
		}
		else
		{
			_log("[Controller] onInitial() Create CClientMeetingAgent Success. Port: %d, MsqKey: %d", nPort, mnMsqKey);
		}
	}
	else
	{
		_log("[Controller] onInitial() config->loadConfig(strConfPath) failed");
	}

	delete config;
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
		_log("[Controller] EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE");
		mCClientMeetingAgent->onReceive(nId, pData);
		break;

	case EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT:
		_log("[Controller] EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT");
		// TODO 重新連線
		// TODO startClientMeetingAgent()?????????????
		// TODO 用 enquire link 
		break;

	default:
		_log("[Controller] Unknown message command: %d", nCommand);
		break;
	}
}

void CController::onHandleMessage(Message &message)
{

}

int CController::startClientMeetingAgent(string strIP, const int nPort, const int nMsqId)
{
	_log("[Controller] Connecting to Meeting-Agent server, IP: %s, port: %d", strIP.c_str(), nPort);
	return mCClientMeetingAgent->startClient(strIP, nPort, nMsqId);
}
