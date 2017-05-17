#include "CClientMeetingAgent.h"

#include "common.h"
#include "event.h"
#include "packet.h"
#include "AndroidPackageInfoQuierer.hpp"
#include "CClientAmxController.h"
#include "CCmpHandler.h"
#include "CConfig.h"
#include "CDataHandler.cpp"
#include "CThreadHandler.h"
#include "ICallback.h"
#include "IReceiver.h"
#include "JSONObject.h"
#include "TestStringsDefinition.h"
#include "UserAppVersionHandler/UserAppVersionHandler.h"
#include "UserAppVersionHandler/UserApkPeekingAppVersionHandler.h"
#include "UserAppVersionHandler/UserConfigFileAppVersionHandler.h"

#define CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG "APP DOWNLOAD INFO CONFIG WATCHER"
#define CONF_BLOCK_AMX_CONTROLLER "CLIENT AMX CONTROLLER"

CClientMeetingAgent::CClientMeetingAgent() :
	CSocketClient(), cmpParser(CCmpHandler::getInstance())
{
	mapFunc[bind_response] = &CClientMeetingAgent::cmpBindResponse;
	mapFunc[unbind_response] = &CClientMeetingAgent::cmpUnbindResponse;

	mapFunc[smart_building_qrcode_tokn_request] = &CClientMeetingAgent::cmpQRCodeToken;
	mapFunc[smart_building_appversion_request] = &CClientMeetingAgent::cmpAPPVersion;
	mapFunc[smart_building_getmeetingdata_request] = &CClientMeetingAgent::cmpGetMeetingData;
	mapFunc[smart_building_amx_control_access_request] = &CClientMeetingAgent::cmpAMXControlAccess;
}

CClientMeetingAgent::~CClientMeetingAgent()
{
	stopClient();
}

int CClientMeetingAgent::initMember(std::unique_ptr<CConfig>& config)
{
	if (initUserAppVersionHandler(config) == FALSE)
	{
		return FALSE;
	}

	string strAmxControllerIp = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "server_ip");
	string strAmxControllerUserControlPort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "user_port");
	string strAmxControllerValidationPort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "validation_port");
	if (strAmxControllerIp.empty() || strAmxControllerUserControlPort.empty()
		|| strAmxControllerValidationPort.empty())
	{
		_log("[CController] onInitial(): AMX controller client config 404");
		return FALSE;
	}

	int amxControllerUserControlPort;
	int amxControllerValidationPort;
	convertFromString(amxControllerUserControlPort, strAmxControllerUserControlPort);
	convertFromString(amxControllerValidationPort, strAmxControllerValidationPort);
	amxControllerClient.reset(new CClientAmxController(strAmxControllerIp,
		amxControllerUserControlPort, amxControllerValidationPort));

	return doorAccessHandler.initMember(config);
}

// 根據 config 檔有甚麼可以用的就初始化其中一種 UserAppVersionHandler
int CClientMeetingAgent::initUserAppVersionHandler(std::unique_ptr<CConfig> &config)
{
	string strAaptPath = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "aapt_path");
	string strApkDir = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "apk_dir");
	string strPkgName = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "package_name");
	string strDownloadLinkBase = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "download_link_base");

	if (!strAaptPath.empty() && !strApkDir.empty()
		&& !strPkgName.empty() && !strDownloadLinkBase.empty())
	{
		_log("[CController] onInitial(): init UserApkPeekingAppVersionHandler");
		auto apkQuierer = new AndroidPackageInfoQuierer(strAaptPath, strPkgName);
		userAppVersionHandler.reset(new UserApkPeekingAppVersionHandler(
			apkQuierer, strPkgName, strApkDir, strDownloadLinkBase));
		return TRUE;
	}

	string strAppDownloadLinkConfigDir = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "config_dir");
	string strAppDownloadLinkConfigName = config->getValue(CONF_BLOCK_APP_DOWNLOAD_INFO_CONFIG, "config_name");

	if (!strAppDownloadLinkConfigDir.empty()
		&& !strAppDownloadLinkConfigName.empty())
	{
		_log("[CController] onInitial(): init UserConfigFileAppVersionHandler");
		userAppVersionHandler.reset(new UserConfigFileAppVersionHandler(
			strAppDownloadLinkConfigDir, strAppDownloadLinkConfigName));
		return TRUE;
	}

	_log("[CController] onInitial(): init AppVersionHandler cannot be instantiated");
	return FALSE;
}

int CClientMeetingAgent::startClient(string strIP, const int nPort, const int nMsqId)
{
	_log("[CClientMeetingAgent] startClient() server IP: %s, port: %d, msqId: %d", strIP.c_str(), nPort, nMsqId);

	if (0 >= nPort || 0 >= nMsqId)
	{
		return FALSE;
	}

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
		_log("[CClientMeetingAgent] Socket Creation Failed");
		return FALSE;
	}
	else
	{
		_log("[CClientMeetingAgent] Connect to Controller-MeetingAgent OK!!");
		_log("[CClientMeetingAgent] send bind request");
		this->cmpBindRequest();
	}

	if (userAppVersionHandler != nullptr)
	{
		userAppVersionHandler->start();
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
			_log("[CServerMeeting] command %s, seqNum is %d, data: %s\n", numberToHex(commandID).c_str(), seqNum, bodyData.c_str());
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
			_log("[CServerMeeting] command %s, seqNum is %d\n", numberToHex(commandID).c_str(), seqNum);
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
	_log("[CClientMeetingAgent] stopClient() step in");

	cmpUnbindRequest();
	stop();

	if (this->userAppVersionHandler != nullptr)
	{
		userAppVersionHandler->stop();
	}

	if (amxControllerClient != nullptr)
	{
		//amxControllerClient->stop();
	}

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


	if (htonl(cmpPacket->cmpHeader.command_length) > (int)sizeof(CMP_HEADER))
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
	else
	{
		// fail them!
		//sendCommand(generic_nack | nCommand, nSequence, strResponseBodyData);
	}

	return TRUE;
}

int CClientMeetingAgent::cmpAPPVersion(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_DBG("[CClientMeetingAgent] cmpAPPVersion");
	string bodyData;

	if (this->userAppVersionHandler == nullptr)
	{
		bodyData =
			R"({"VERSION": "0.0.0", "VERSION_CODE": 0, "APP_DOWNLOAD_URL": ""})";
	} 
	else
	{
		bodyData =
			"{\"VERSION\": \"" + userAppVersionHandler->getVersionName()
			+ "\", \"VERSION_CODE\": " + std::to_string(userAppVersionHandler->getVersionCode())
			+ ", \"APP_DOWNLOAD_URL\": \"" + userAppVersionHandler->getDownloadLink() + "\"}";
	}

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
	const CMP_PACKET * cmpPacket = reinterpret_cast<const CMP_PACKET *>(pData);
	_log("[ClientMeetingAgent] In CMPAMXControlAccess get body: %s", cmpPacket->cmpBody.cmpdata);

	if (htonl(cmpPacket->cmpHeader.command_length) > 16)
	{
		_log("[ClientMeetingAgent] In CMPQRcodeToken get body: %s", cmpPacket->cmpBody.cmpdata);
		string strRequestBodyData = string(cmpPacket->cmpBody.cmpdata);
		string strResponseBodyData = "";

		std::stringstream ss;

		if (strRequestBodyData.find(TEST_USER_HAS_MEETING_IN_001) != string::npos
			&& strRequestBodyData.find("ITES_101") != string::npos)
		{
			//OK can control AMX
			ss << "{\"USER_ID\": \"" << TEST_USER_HAS_MEETING_IN_001
				<< "\", \"RESULT\": true, \"ROOM_IP\": \"" << amxControllerClient->getServerIp()
				<< "\", \"ROOM_PORT\": " << amxControllerClient->getUserPort()
				<< ", \"ROOM_TOKEN\": \"" << TEST_AMX_TOKEN << "\"}";
			strResponseBodyData = ss.str();
		}
		else
		{
			//NO can not control AMX
			ss << "{\"USER_ID\": \"" << TEST_USER_HAS_MEETING_IN_001
				<< "\", \"RESULT\": false}";
			strResponseBodyData = ss.str();
		}

		sendCommand(generic_nack | nCommand, nSequence, strResponseBodyData);
	}
	else 
	{
		// pdu bad size, send reject
	}

	return TRUE;
}

int CClientMeetingAgent::cmpBindResponse(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_log("[CClientMeetingAgent] cmpBind Response");

	return TRUE;
}

int CClientMeetingAgent::cmpUnbindResponse(int nSocket, int nCommand, int nSequence, const void *pData)
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
		_log("[CClientMeetingAgent] cmpUnbindRequest() send command!");
		sendCommand(unbind_request, getSequence(), "");
	}
	else
	{
		_log("[CClientMeetingAgent] cmpUnbindRequest() ERROR while send unBind Request!");
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
