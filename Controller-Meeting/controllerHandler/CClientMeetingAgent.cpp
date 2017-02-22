#include "IReceiver.h"
#include "event.h"
#include "common.h"
#include "CClientMeetingAgent.h"
#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "JSONObject.h"
#include "ICallback.h"
#include "packet.h"

static CClientMeetingAgent * clientMeetingAgent = 0;

CClientMeetingAgent::CClientMeetingAgent() :
		CSocketClient(), cmpParser(CCmpHandler::getInstance())
{

	mapFunc[bind_response] = &CClientMeetingAgent::cmpBind;
	mapFunc[unbind_response] = &CClientMeetingAgent::cmpUnbind;

	mapFunc[smart_building_qrcode_tokn_request] = &CClientMeetingAgent::cmpQRCodeToken;
	mapFunc[smart_building_appversion_request] = &CClientMeetingAgent::cmpAPPVersion;
	mapFunc[smart_building_getmeetingdata_request] = &CClientMeetingAgent::cmpGetMeetingData;
	mapFunc[smart_building_amx_control_access_request] = &CClientMeetingAgent::cmpAMXControlAccess;
}

CClientMeetingAgent::~CClientMeetingAgent()
{
	stop();
}

CClientMeetingAgent * CClientMeetingAgent::getInstance()
{
	if (0 == clientMeetingAgent)
	{
		clientMeetingAgent = new CClientMeetingAgent();
	}
	return clientMeetingAgent;
}

int CClientMeetingAgent::startClient(string strIP, const int nPort, const int nMsqId)
{
	_DBG("[CClientMeetingAgent] startClient");
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if (0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_MEETING_AGENT_RECEIVE);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	setPacketConf(PK_CMP, PK_MSQ);

	const char* cszAddr = NULL;
	if (!strIP.empty())
	{
		cszAddr = strIP.c_str();
	}

	start(AF_INET, cszAddr, nPort);
	if (!this->isValidSocketFD())
	{
		_log("[CClientMeetingAgent] Socket Create Fail");
		return FALSE;
	}
	else
	{
		_log("[CClientMeetingAgent] Connect Controller-MeetingAgent Success!!");

		_DBG("[CClientMeetingAgent] now bind request");
		this->cmpBindRequest();

	}

	return TRUE;
}

int CClientMeetingAgent::sendCommand(int commandID, int seqNum, string bodyData)
{
	if (bodyData.size() > 0)
	{
		_log("[CServerMeeting] command %d, seqNum is %d, data: %s\n", commandID, seqNum, bodyData.c_str());
	}
	else
	{
		_log("[CServerMeeting] command %d, seqNum is %d\n", commandID, seqNum);
	}
	vector<int>::iterator it;

	int nSocket = -1;
	if (clientMeetingAgent->isValidSocketFD())
	{
		nSocket = clientMeetingAgent->getSocketfd();
	}

	int nRet = 0;

	if (nSocket >= 0)
	{
		if (bodyData.size() > 0)
		{
			nRet = sendPacket(dynamic_cast<CSocket*>(clientMeetingAgent), nSocket, commandID, STATUS_ROK, seqNum,
					bodyData.c_str());
		}
		else
		{
			nRet = sendPacket(dynamic_cast<CSocket*>(clientMeetingAgent), nSocket, commandID, STATUS_ROK, seqNum, 0);
		}
	}
	else
	{
		_log("[CServerMeeting] ERROR to find Controller-Meeting Socket ID!");
	}
	return nRet;
}

void CClientMeetingAgent::stopClient()
{
	stop();
}

void CClientMeetingAgent::onReceive(const int nSocketFD, const void *pData)
{
	int nRet = -1;
	int nPacketLen = 0;
	CMP_HEADER cmpHeader;
	char *pPacket;

	pPacket = (char*) const_cast<void*>(pData);
	memset(&cmpHeader, 0, sizeof(CMP_HEADER));

	cmpHeader.command_id = cmpParser->getCommand(pPacket);
	cmpHeader.command_length = cmpParser->getLength(pPacket);
	cmpHeader.command_status = cmpParser->getStatus(pPacket);
	cmpHeader.sequence_number = cmpParser->getSequence(pPacket);

	printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
			"[CClientMeetingAgent] Recv ", nSocketFD);

	if (cmpParser->isAckPacket(cmpHeader.command_id))
	{
		return;
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if (0x000000FF < cmpHeader.command_id || 0x00000000 >= cmpHeader.command_id || mapFunc.end() == iter)
	{
		sendPacket(dynamic_cast<CSocket*>(clientMeetingAgent), nSocketFD, generic_nack | cmpHeader.command_id,
		STATUS_RINVCMDID, cmpHeader.sequence_number, 0);

		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);

}

int CClientMeetingAgent::cmpQRCodeToken(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CClientMeetingAgent]cmpQRCodeToken");

	string bodyData = "";





	sendCommand(generic_nack | nCommand, nSequence, bodyData);
	return TRUE;
}

int CClientMeetingAgent::cmpAPPVersion(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CClientMeetingAgent]cmpAPPVersion");

	string bodyData =
			"{\"VERSION\": \"1.0\",\"APP_DOWNLOAD_URL\":  \"http://XXX/ideas/sdk/download/libs/android/XXX.apk\"}";

	sendCommand(generic_nack | nCommand, nSequence, bodyData);

	return TRUE;
}

int CClientMeetingAgent::cmpGetMeetingData(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CClientMeetingAgent]cmpGetMeetingData");

	string bodyData = "";

	sendCommand(generic_nack | nCommand, nSequence, bodyData);

	return TRUE;
}

int CClientMeetingAgent::cmpAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CClientMeetingAgent]cmpAMXControlAcess");

	string bodyData = "";

	sendCommand(generic_nack | nCommand, nSequence, bodyData);

	return TRUE;
}

int CClientMeetingAgent::cmpBind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_log("[CClientMeetingAgent] cmpBind Response");

	return TRUE;
}

int CClientMeetingAgent::cmpUnbind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_log("[CClientMeetingAgent] cmpUnbind Response");

	return TRUE;
}

void CClientMeetingAgent::cmpBindRequest()
{
	if (this->isValidSocketFD())
	{
		sendCommand(bind_request, getSequence(), "");
	}
	else
	{
		_log("[CClientMeetingAgent] ERROR while send Bind Request!");
	}

}

void CClientMeetingAgent::cmpUnbindRequest()
{
	if (this->isValidSocketFD())
	{
		sendCommand(unbind_request, getSequence(), "");
	}
	else
	{
		_log("[CClientMeetingAgent] ERROR while send unBind Request!");
	}

}


CMPData CClientMeetingAgent::parseCMPData(int nSocket, int nCommand, int nSequence, const void *pData, bool isBodyExist)
{
	CDataHandler<string> rData;

	int nRet = cmpParser->parseBody(nCommand, pData, rData);

	if (0 < nRet && rData.isValidKey("data"))
	{
		return CMPData(nSocket, nCommand, nSequence, rData["data"]);
	}
	else if (isBodyExist == false)
	{
		return CMPData(nSocket, nCommand, nSequence, "");
	}
	else
	{
		return CMPData(-1, -1, -1, NULL);
	}
}

