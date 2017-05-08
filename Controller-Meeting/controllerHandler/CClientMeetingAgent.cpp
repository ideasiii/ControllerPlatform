#include "IReceiver.h"
#include "event.h"
#include "common.h"
#include "CClientMeetingAgent.h"
#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "TestStringsDefinition.h"
#include "JSONObject.h"
#include "ICallback.h"
#include "packet.h"

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

int CClientMeetingAgent::startClient(string strIP, const int nPort, const int nMsqId)
{
	_DBG("[CClientMeetingAgent] startClient()");
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket client client to controller-meetingagent **/
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

	vector<int>::iterator it;

	int nSocket = -1;
	if (isValidSocketFD())
	{
		nSocket = getSocketfd();
	}

	int nRet = 0;

	if (nSocket >= 0)
	{
		if (bodyData.size() > 0)
		{
			_log("[CServerMeeting] command %d, seqNum is %d, data: %s\n", commandID, seqNum, bodyData.c_str());
			if (bodyData.size() > MAX_SIZE - 17)
			{
				nRet = sendPacket(dynamic_cast<CSocket*>(this), nSocket, commandID, STATUS_ROK, seqNum,
					bodyData.c_str(), true);
			}
			else
			{
				nRet = sendPacket(dynamic_cast<CSocket*>(this), nSocket, commandID, STATUS_ROK, seqNum,
					bodyData.c_str());
			}
		}
		else
		{
			_log("[CServerMeeting] command %d, seqNum is %d\n", commandID, seqNum);
			nRet = sendPacket(dynamic_cast<CSocket*>(this), nSocket, commandID, STATUS_ROK, seqNum, 0);
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
		sendPacket(dynamic_cast<CSocket*>(this), nSocketFD, generic_nack | cmpHeader.command_id,
			STATUS_RINVCMDID, cmpHeader.sequence_number, 0);

		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);

}

int CClientMeetingAgent::cmpQRCodeToken(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CClientMeetingAgent]cmpQRCodeToken");
	const CMP_PACKET * cmpPacket = reinterpret_cast<const CMP_PACKET *>(pData);

	if (htonl(cmpPacket->cmpHeader.command_length) > 16)
	{
		_log("[ClientMeetingAgent] In CMPQRcodeToken get body:%s", cmpPacket->cmpBody.cmpdata);
		string strRequestBodyData = string(cmpPacket->cmpBody.cmpdata);
		string strResponseBodyData = "";

		if (strRequestBodyData.find(
			"U9oKcId0/8PgoYnXpNaKq3/juvgr4C8HlIk82lF/FazsezL3D54oD3ioZtYtS6PRMwcShS+nvXrtREn4gqfKLw==")
			!= string::npos)
		{
			//AMX Permission User
			strResponseBodyData =
				"{\"QRCODE_TYPE\": \"1\",\"MESSAGE\":{\"USER_ID\": \"00000000-ffff-0000-ffff-ffffffffffff\"}}";
		}
		else if (strRequestBodyData.find(
			"SJlze0jJqfG7IrShMx7e0XoZ6LWdfKFE4G4i9yk2I/m9kumXDesRLQOEcTevE/FlkJHMOVBcaSj6XmZ9QtF3KA==")
			!= string::npos)
		{
			//other user
			strResponseBodyData =
				"{\"QRCODE_TYPE\": \"1\",\"MESSAGE\":{\"USER_ID\": \"ffffffff-ffff-0000-0000-ffffffffffff\"}}";

		}
		else if (strRequestBodyData.find(
			"m+eJYbDinOt7XGXfVdBw5EZhQDDgmpnF8HXQr3Nkj6LBMSF+aGmoW//g54AXQcmrKl+gzOm4pz71tCLKOmR55g==")
			!= string::npos)
		{
			//digit sign up
			if (strRequestBodyData.find("00000000-ffff-0000-ffff-ffffffffffff") != string::npos)
			{
				strResponseBodyData =
					"{\"QRCODE_TYPE\": \"2\",\"MESSAGE\":{\"RESULT\":true,\"RESULT_MESSAGE\":\"Sign Up Successful\"}}";
			}
			else
			{
				strResponseBodyData =
					"{\"QRCODE_TYPE\":\"2\",\"MESSAGE\":{\"RESULT\":false,\"RESULT_MESSAGE\": \"You Have No Meeting Today\"}}";
			}

		}
		else if (strRequestBodyData.find(DOOR_QR_CODE_101_DUMMY) != string::npos
			|| strRequestBodyData.find(DOOR_QR_CODE_102_DUMMY) != string::npos
			|| strRequestBodyData.find(DOOR_QR_CODE_101) != string::npos
			|| strRequestBodyData.find(DOOR_QR_CODE_102) != string::npos)
		{
			// Door access
			std::string uuid;
			if (strRequestBodyData.find(TEST_USER_HAS_MEETING_IN_001) != string::npos)
			{
				uuid = TEST_USER_HAS_MEETING_IN_001;
			}
			else if (strRequestBodyData.find(TEST_USER_HAS_MEETING_IN_002) != string::npos)
			{
				uuid = TEST_USER_HAS_MEETING_IN_002;
			}

			if (uuid.size() > 0)
			{
				std::string meetingRoom = "000";

				if (strRequestBodyData.find(DOOR_QR_CODE_101_DUMMY) != string::npos
					|| strRequestBodyData.find(DOOR_QR_CODE_101) != string::npos)
				{
					meetingRoom = "101";
				}
				else if (strRequestBodyData.find(DOOR_QR_CODE_102_DUMMY) != string::npos
					|| strRequestBodyData.find(DOOR_QR_CODE_102) != string::npos)
				{
					meetingRoom = "102";
				}

				if (strRequestBodyData.find(DOOR_QR_CODE_101_DUMMY) != string::npos
					|| strRequestBodyData.find(DOOR_QR_CODE_102_DUMMY) != string::npos)
				{
					strResponseBodyData = doorAccessHandler.doRequestDummy(uuid, meetingRoom);
				}
				else
				{
					strResponseBodyData = doorAccessHandler.doRequest(uuid, meetingRoom);
				}
			}
			else
			{
				strResponseBodyData =
					"{\"QRCODE_TYPE\":\"3\",\"MESSAGE\":{\"RESULT\":false,\"RESULT_MESSAGE\":\"No permission to open this door\"}}";
			}
		}
		else
		{
			strResponseBodyData =
				"{\"QRCODE_TYPE\":\"0\",\"MESSAGE\":{\"RESULT_MESSAGE\":\"Unknown this QR-Code Type\"}}";
		}

		sendCommand(generic_nack | nCommand, nSequence, strResponseBodyData);
	}
	return TRUE;
}

int CClientMeetingAgent::cmpAPPVersion(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CClientMeetingAgent] cmpAPPVersion");

	string bodyData =
		"{\"VERSION\": \"1.0\",\"APP_DOWNLOAD_URL\": \"http://XXX/ideas/sdk/download/libs/android/XXX.apk\"}";

	sendCommand(generic_nack | nCommand, nSequence, bodyData);

	return TRUE;
}

int CClientMeetingAgent::cmpGetMeetingData(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CClientMeetingAgent]cmpGetMeetingData");
	const CMP_PACKET * cmpPacket = reinterpret_cast<const CMP_PACKET *>(pData);

	if (htonl(cmpPacket->cmpHeader.command_length) > 16)
	{
		_log("[ClientMeetingAgent] In CMPGetMeetingData get body: %s", cmpPacket->cmpBody.cmpdata);
		string strRequestBodyData = string(cmpPacket->cmpBody.cmpdata);
		string strResponseBodyData = "";

		if (strRequestBodyData.find("00000000-ffff-0000-ffff-ffffffffffff") != string::npos)
		{
			strResponseBodyData =
				"{\"USER_ID\":\"00000000-ffff-0000-ffff-ffffffffffff\",\"USER_NAME\":\"李二二\",\"USER_EMAIL\":\"qwwwew@gmail.com\",\"MEETING_DATA\":[{\"MEETING_ID\":\"a46595d0-fbcd-4d56-8bdc-3d8fa659b6a1\",\"SUPJECT\":\"XXX公司會議\",\"START_TIME\":\"2016-06-30 09:30:00\",\"END_TIME\":\"2016-06-30 12:30:00\",\"ROOM_ID\":\"ITES_101\",\"OWNER\":\"王一一\",\"OWNER_EMAIL\":\"qwer1234@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1afd78\",\"SUPJECT\":\"促進XXX發展計畫\",\"START_TIME\":\"2016-07-30 09:30:00\",\"END_TIME\":\"2016-07-30 12:30:00\",\"ROOM_ID\":\"ITES_102\",\"OWNER\":\"王一二\",\"OWNER_EMAIL\":\"qoiu1234@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass78\",\"SUPJECT\":\"nnnn公司會議\",\"START_TIME\":\"2016-08-30 09:30:00\",\"END_TIME\":\"2016-08-30 12:30:00\",\"ROOM_ID\":\"ITES_103\",\"OWNER\":\"王一三\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass71\",\"SUPJECT\":\"促進WWWW發展計畫\",\"START_TIME\":\"2016-08-31 09:30:00\",\"END_TIME\":\"2016-08-31 12:30:00\",\"ROOM_ID\":\"ITES_102\",\"OWNER\":\"王一四\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass72\",\"SUPJECT\":\"促進YYY發展計畫\",\"START_TIME\":\"2016-09-30 09:30:00\",\"END_TIME\":\"2016-09-30 12:30:00\",\"ROOM_ID\":\"ITES_103\",\"OWNER\":\"王一五\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46c0-b0c0-00eede1ass73\",\"SUPJECT\":\"XXXx公司會議\",\"START_TIME\":\"2016-10-30 09:30:00\",\"END_TIME\":\"2016-10-30 12:30:00\",\"ROOM_ID\":\"ITES_103\",\"OWNER\":\"王一六\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-43b0-b0c0-00eede1ass74\",\"SUPJECT\":\"促進YXXY發展計畫\",\"START_TIME\":\"2016-11-28 09:30:00\",\"END_TIME\":\"2016-11-28 12:30:00\",\"ROOM_ID\":\"ITES_104\",\"OWNER\":\"王一七\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass75\",\"SUPJECT\":\"促進WWXY發展計畫\",\"START_TIME\":\"2016-11-29 09:30:00\",\"END_TIME\":\"2016-11-29 12:30:00\",\"ROOM_ID\":\"ITES_104\",\"OWNER\":\"王一八\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass76\",\"SUPJECT\":\"促進WWY發展計畫\",\"START_TIME\":\"2016-11-30 09:30:00\",\"END_TIME\":\"2016-11-30 12:30:00\",\"ROOM_ID\":\"ITES_104\",\"OWNER\":\"王一九\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass18\",\"SUPJECT\":\"促進YYYX發展計畫\",\"START_TIME\":\"2016-12-30 09:30:00\",\"END_TIME\":\"2016-12-30 12:30:00\",\"ROOM_ID\":\"ITES_103\",\"OWNER\":\"王一十\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass28\",\"SUPJECT\":\"促進YYYW發展計畫\",\"START_TIME\":\"2016-12-31 09:30:00\",\"END_TIME\":\"2016-12-31 12:30:00\",\"ROOM_ID\":\"ITES_102\",\"OWNER\":\"王二一\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"}]}";

		}
		else if (strRequestBodyData.find("ffffffff-ffff-0000-0000-ffffffffffff") != string::npos)
		{
			strResponseBodyData =
				"{\"USER_ID\":\"ffffffff-ffff-0000-0000-ffffffffffff\",\"USER_NAME\":\"王二二\",\"USER_EMAIL\":\"qqdsdw@iii.org.tw\",\"MEETING_DATA\":[{\"MEETING_ID\":\"a46595d0-fbcd-4d56-8bdc-3d8fa659b6a1\",\"SUPJECT\":\"XXX公司會議\",\"START_TIME\":\"2017-06-30 09:30:00\",\"END_TIME\":\"2017-06-30 12:30:00\",\"ROOM_ID\":\"ITES_101\",\"OWNER\":\"王一一\",\"OWNER_EMAIL\":\"qwer1234@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass78\",\"SUPJECT\":\"nnnn公司會議\",\"START_TIME\":\"2017-08-30 09:30:00\",\"END_TIME\":\"2017-08-30 12:30:00\",\"ROOM_ID\":\"ITES_103\",\"OWNER\":\"王一三\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"}]}";

		}
		else
		{
			strResponseBodyData = "{\"USER_ID\":\"UNKOWN USER\"}";
		}

		sendCommand(generic_nack | nCommand, nSequence, strResponseBodyData);
	}

	return TRUE;
}

int CClientMeetingAgent::cmpAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CClientMeetingAgent]cmpAMXControlAcess");
	const CMP_PACKET * cmpPacket = reinterpret_cast<const CMP_PACKET *>(pData);
	_log("[ClientMeetingAgent] In CMPAMXControlAccess get body: %s", cmpPacket->cmpBody.cmpdata);

	if (htonl(cmpPacket->cmpHeader.command_length) > 16)
	{
		_log("[ClientMeetingAgent] In CMPQRcodeToken get body:%s", cmpPacket->cmpBody.cmpdata);
		string strRequestBodyData = string(cmpPacket->cmpBody.cmpdata);
		string strResponseBodyData = "";

		if (strRequestBodyData.find("00000000-ffff-0000-ffff-ffffffffffff") != string::npos
			&& strRequestBodyData.find("ITES_101") != string::npos)
		{
			//OK can control AMX
			strResponseBodyData =
				"{\"USER_ID\": \"00000000-ffff-0000-ffff-ffffffffffff\",\"RESULT\": true,\"ROOM_IP\": \"54.199.198.94\",\"ROOM_PORT\": 2309,\"ROOM_TOKEN\": \"28084ca1-7386-4fa6-b174-098ee2784a5d\"}";

		}
		else
		{
			//NO can not control AMX
			strResponseBodyData = "{\"USER_ID\": \"ffffffff-ffff-0000-0000-ffffffffffff\",\"RESULT\": false}";
		}

		sendCommand(generic_nack | nCommand, nSequence, strResponseBodyData);
	}

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
		_log("[CClientMeetingAgent] parseCMP Data");
		return CMPData(nSocket, nCommand, nSequence, rData["data"]);
	}
	else if (isBodyExist == false)
	{
		_log("[CClientMeetingAgent] parseCMP Data NO Body");
		return CMPData(nSocket, nCommand, nSequence, "");
	}
	else
	{
		_log("[CClientMeetingAgent] parseCMP Data ERROR");
		return CMPData(-1, -1, -1, "");
	}
}
