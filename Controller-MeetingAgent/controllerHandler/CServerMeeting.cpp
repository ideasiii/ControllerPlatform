#include "CSocketServer.h"
#include "event.h"
#include "packet.h"
#include "common.h"
#include "CDataHandler.cpp"
#include "CServerMeeting.h"
#include "IReceiver.h"
#include "utility.h"
#include <algorithm>

#include "CCmpHandler.h"

static CServerMeeting * serverMeeting = 0;

/** Enquire link function declare for enquire link thread **/
void *threadEnquireLinkRequest(void *argv);

CServerMeeting::CServerMeeting() :
		CSocketServer(), cmpParser(CCmpHandler::getInstance()), tdEnquireLink(new CThreadHandler)
{

	mapFunc[bind_request] = &CServerMeeting::cmpBind;
	mapFunc[unbind_request] = &CServerMeeting::cmpUnbind;

	mapFunc[smart_building_qrcode_tokn_response] = &CServerMeeting::cmpQRCodeToken;
	mapFunc[smart_building_appversion_response] = &CServerMeeting::cmpAPPVersion;
	mapFunc[smart_building_getmeetingdata_response] = &CServerMeeting::cmpGetMeetingData;
	mapFunc[smart_building_amx_control_access_response] = &CServerMeeting::cmpAMXControlAccess;

	mapFunc[enquire_link_response] = &CServerMeeting::cmpEnquireLinkResponse;

}

CServerMeeting::~CServerMeeting()
{
	stopServer();
}

void CServerMeeting::onReceive(const int nSocketFD, const void *pData)
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
			"[CServerMeeting] ### Recv ", nSocketFD);

	//response message get!
	if (cmpParser->isAckPacket(cmpHeader.command_id))
	{
		(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number,
				pPacket);
		return;
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if ((0x000000FF > cmpHeader.command_id && 0x00000000 <= cmpHeader.command_id) || mapFunc.end() != iter)
	{

	}
	else
	{
		sendPacket(dynamic_cast<CSocket*>(serverMeeting), nSocketFD, generic_nack | cmpHeader.command_id,
		STATUS_RINVCMDID, cmpHeader.sequence_number, 0);

		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);

}

int CServerMeeting::controllerCallBack(int nSocketFD, int nDataLen, const void *pData)
{

	_log("[CServerMeeting] start on Controller Call Back Func");

	const CMP_PACKET * cmpPacket = reinterpret_cast<const CMP_PACKET *>(pData);

	(this->*this->mapFunc[ntohl(cmpPacket->cmpHeader.command_id)])(nSocketFD, ntohl(cmpPacket->cmpHeader.command_id),
			ntohl(cmpPacket->cmpHeader.sequence_number), static_cast<void*>(const_cast<CMP_PACKET*>(cmpPacket)));
	_log("[CServerMeeting] end on Controller Call Back Func");

	return TRUE;
}

int CServerMeeting::cmpQRCodeToken(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CServerMeeting]cmpQRCodeToken");

	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, pData, true);
	if (mCMPData.isVaild())
	{
		(*mapCallback[CB_DEVCIE_COMMAND])(static_cast<void*>(const_cast<CMPData*>(&mCMPData)));
	}
	else
	{
		return FALSE;
	}
	return TRUE;

}

int CServerMeeting::cmpAPPVersion(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CServerMeeting]cmpAPPVersion");

	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, pData, true);
	if (mCMPData.isVaild())
	{
		(*mapCallback[CB_DEVCIE_COMMAND])(static_cast<void*>(const_cast<CMPData*>(&mCMPData)));
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

int CServerMeeting::cmpGetMeetingData(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CServerMeeting]cmpGetMeetingData");

	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, pData, true);
	if (mCMPData.isVaild())
	{
		(*mapCallback[CB_DEVCIE_COMMAND])(static_cast<void*>(const_cast<CMPData*>(&mCMPData)));
	}
	else
	{
		return FALSE;
	}
	return TRUE;

}

int CServerMeeting::cmpAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CServerMeeting]cmpAMXControlAcess");

	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, pData, true);
	if (mCMPData.isVaild())
	{
		(*mapCallback[CB_DEVCIE_COMMAND])(static_cast<void*>(const_cast<CMPData*>(&mCMPData)));
	}
	else
	{
		return FALSE;
	}
	return TRUE;

}

CServerMeeting * CServerMeeting::getInstance()
{
	if (0 == serverMeeting)
	{
		serverMeeting = new CServerMeeting();
	}
	return serverMeeting;
}

int CServerMeeting::startServer(string strIP, const int nPort, const int nMsqId)
{
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if (0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_MEETING_RECEIVE);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_MEETING);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_MEETING);
	}

	setPacketConf(PK_CMP, PK_MSQ);

	const char* cszAddr = NULL;
	if (!strIP.empty())
	{
		cszAddr = strIP.c_str();
	}

	if ( FAIL == start( AF_INET, cszAddr, nPort))
	{
		_log("[CServerMeeting] Socket Create Fail");
		return FALSE;
	}

	tdEnquireLink->createThread(threadEnquireLinkRequest, this);

	return TRUE;
}

int CServerMeeting::sendCommand(int commandID, int seqNum, string bodyData)
{
	if (bodyData.size() > 0)
	{
		_log("[CServerMeeting] send command %d, seqNum is %d, data: %s\n", commandID, seqNum, bodyData.c_str());
	}
	else
	{
		_log("[CServerMeeting] send command %d, seqNum is %d\n", commandID, seqNum);
	}
	vector<int>::iterator it;

	int nSocket = -1;

	if (mapClient.size() > 0)
	{
		nSocket = mapClient.front();
	}
	int nRet = 0;

	if (nSocket >= 0)
	{
		if (bodyData.size() > 0)
		{
			nRet = sendPacket(dynamic_cast<CSocket*>(serverMeeting), nSocket, commandID, STATUS_ROK, seqNum,
					bodyData.c_str());
		}
		else
		{
			nRet = sendPacket(dynamic_cast<CSocket*>(serverMeeting), nSocket, commandID, STATUS_ROK, seqNum, 0);
		}
	}
	else
	{
		_log("[CServerMeeting] ERROR to find Controller-Meeting Socket ID!");
	}
	return nRet;
}

int CServerMeeting::cmpBind(int nSocket, int nCommand, int nSequence, const void *pData)
{

	mapClient.push_back(nSocket);
	_log("[CServerMeeting] Socket Controller-Meeting Client FD:%d Binded", nSocket);
	sendPacket(dynamic_cast<CSocket*>(serverMeeting), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, 0);

	return TRUE;
}

int CServerMeeting::cmpUnbind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	deleteClient(nSocket);
	sendPacket(dynamic_cast<CSocket*>(serverMeeting), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, 0);
	return TRUE;
}

void CServerMeeting::stopServer()
{
	stop();
}

void CServerMeeting::addClient(const int nSocketFD)
{
	_log("[CServerMeeting] Socket Client FD:%d Connected", nSocketFD);
}

void CServerMeeting::deleteClient(const int nSocketFD)
{
	vector<int>::iterator position = find(mapClient.begin(), mapClient.end(), nSocketFD);

	if (position != mapClient.end())
	{
		mapClient.erase(position);
		_log("[CServerMeeting] Socket Client FD:%d UnBinded", nSocketFD);
	}

	_log("[CServerMeeting] Socket Client FD:%d Disconnected", nSocketFD);
}

CMPData CServerMeeting::parseCMPData(int nSocket, int nCommand, int nSequence, const void *pData, bool isBodyExist)
{

	_log("[CServerMeeting] parseCMPData: (nCommand, nSequence) = (%d, %d)", nCommand, nSequence);

	const CMP_PACKET * cmpPacket = reinterpret_cast<const CMP_PACKET *>(pData);

	if (ntohl(cmpPacket->cmpHeader.command_length) > MAX_SIZE - 1)
	{

		_log("[CServerMeeting::parseCMPData] cmpPacket body: %s", cmpPacket->cmpBodyUnlimit.cmpdata);
		return CMPData(nSocket, nCommand, nSequence, string(cmpPacket->cmpBodyUnlimit.cmpdata));
	}

	CDataHandler<string> rData;

	int nRet = cmpParser->parseBody(nCommand, pData, rData);

	_log("[CServerMeeting] nRet: %d\n", nRet);

	_log("[CServerMeeting] rData.isValidKey:%s\n", rData.isValidKey("data") ? "true" : "false");

	if (0 < nRet && rData.isValidKey("data"))
	{
		_log("[CServerMeeting] parseCMP Data");
		return CMPData(nSocket, nCommand, nSequence, rData["data"]);
	}
	else if (isBodyExist == false)
	{
		_log("[CServerMeeting] parseCMP Data NO Body");
		return CMPData(nSocket, nCommand, nSequence, "");
	}
	else
	{
		_log("[CServerMeeting] parseCMP Data ERROR");
		return CMPData(-1, -1, -1, "");
	}
}
void CServerMeeting::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

int CServerMeeting::cmpEnquireLinkResponse(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_log("[CServerMeeting] Get Enquire Link Response!");
	return 1;
}

void CServerMeeting::runEnquireLinkRequest()
{
	int nSocketFD = -1;
	string strSql;
	string strLog;

	while (1)
	{
		tdEnquireLink->threadSleep(600);

		for (int i = 0; i < mapClient.size(); i++)
		{
			nSocketFD = mapClient[i];
			int nRet = cmpEnquireLinkRequest(nSocketFD);

			if (nRet > 0)
			{
				//Enquire Link Success
			}
			else
			{
				//Enquire Link Failed
				_log("[CServerMeeting] Send Enquire Link Failed result = %d\n", nRet);
			}
		}

	}
}

int CServerMeeting::cmpEnquireLinkRequest(const int nSocketFD)
{
	return sendCommand(enquire_link_request, getSequence(), "");
}

/************************************* thread function **************************************/
void *threadEnquireLinkRequest(void *argv)
{
	CServerMeeting* ss = reinterpret_cast<CServerMeeting*>(argv);
	ss->runEnquireLinkRequest();
	return NULL;
}

