#include "CClientMeetingAgent.h"

#include <regex>

#include "../enquireLinkYo/EnquireLinkYo.h"
#include "common.h"
#include "event.h"
#include "iCommand.h"
#include "packet.h"
#include "AndroidPackageInfoQuierer.hpp"
#include "AppVersionHandler/AppVersionHandler.h"
#include "AppVersionHandler/AppVersionHandlerFactory.h"
#include "ClientAmxController/AmxControllerInfo.h"
#include "ClientAmxController/CClientAmxControllerFactory.h"
#include "CConfig.h"
#include "CMysqlHandler.h"
#include "JSONObject.h"
#include "RegexPattern.h"
#include "TestStringsDefinition.h"

#define TASK_NAME "ClientAgent"
#define LOG_TAG "[CClientMeetingAgent]"
#define LOG_TAG_COLORED "[\033[1;31mCClientMeetingAgent\033[0m]"

#define CONF_BLOCK_MEETING_AGENT_CLIENT "CLIENT MEETING_AGENT"
#define JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE R"({"QRCODE_TYPE":"0","MESSAGE":{"RESULT_MESSAGE":"Unknown type of QR-Code"}})"

void ReleaseJSONObject(JSONObject *j)
{
	j->release();
}

CClientMeetingAgent::CClientMeetingAgent(CObject *controller) :
	mpController(controller)
{
	enquireLinkYo.reset(new EnquireLinkYo("ClientAgent.ely", this, 
		EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT, mpController));
}

CClientMeetingAgent::~CClientMeetingAgent()
{
	stopClient();
}

int CClientMeetingAgent::initMember(std::unique_ptr<CConfig>& config)
{
	if (initMeetingAgentServerParams(config) == FALSE)
	{
		return FALSE;
	}

	auto appVerHandlerRet = AppVersionHandlerFactory::createFromConfig(config);
	if (appVerHandlerRet == nullptr)
	{
		_log(LOG_TAG" onInitial(): AppVersionHandler cannot be instantiated");
		return FALSE;
	}

	auto amxControllerInfoRet = CClientAmxControllerFactory::getServerInfoFromConfig(config);
	if (amxControllerInfoRet == nullptr)
	{
		_log(LOG_TAG" onInitial(): AmxControllerInfo cannot be instantiated");
		return FALSE;
	}
	
	appVersionHandler.reset(appVerHandlerRet);
	amxControllerInfo.reset(amxControllerInfoRet);
		
	return doorAccessHandler.initMember(config);
}

int CClientMeetingAgent::initMeetingAgentServerParams(std::unique_ptr<CConfig> &config)
{
	string strServerIp = config->getValue(CONF_BLOCK_MEETING_AGENT_CLIENT, "server_ip");
	string strPort = config->getValue(CONF_BLOCK_MEETING_AGENT_CLIENT, "port");
	
	if (strServerIp.empty() || strPort.empty())
	{
		_log(LOG_TAG" initMeetingAgentServerParams() 404");
		return FALSE;
	}

	int nPort;
	convertFromString(nPort, strPort);
	agentIp = strServerIp;
	agentPort = nPort;

	return TRUE;
}

int CClientMeetingAgent::startClient(int msqKey)
{
	_log(LOG_TAG" Connecting to MeetingAgent %s:%d", agentIp.c_str(), agentPort);

	int nRet = connectWithCallback(agentIp.c_str(), agentPort, msqKey, 
		[](CATcpClient *caller, pthread_t msgRecvTid, pthread_t pktRecvTid) -> void
	{
		pthread_setname_np(msgRecvTid, "AgentMsgRecv");
		pthread_setname_np(pktRecvTid, "AgentPktRecv");
	});

	if (nRet < 0)
	{
		_log(LOG_TAG" startClient() Connecting to agent FAILED");
		return FALSE;
	}

	nRet = request(getSocketfd(), bind_request, STATUS_ROK, getSequence(), NULL);
	if (nRet < 0)
	{
		_log(LOG_TAG" startClient() Binding to agent FAILED");
		return FALSE;
	}

	appVersionHandler->start();
	// enquireLinkYo starts in onResponse(), when binding response is received
	
	return TRUE;
}

void CClientMeetingAgent::stopClient()
{
	//_DBG(LOG_TAG" stopClient() step in");

	if (enquireLinkYo != nullptr)
	{
		enquireLinkYo->stop();
	}

	if (appVersionHandler != nullptr)
	{
		appVersionHandler->stop();
	}

	if (!isValidSocketFD())
	{
		_log(LOG_TAG" stopClient() socket fd is not valid, quit stopping");
		return;
	}

	// server will not response to unbind_request
	// receiving unbind_response while destructing class may cause segmentation fault
	request(getSocketfd(), unbind_request, STATUS_ROK, getSequence(), NULL);
	stop();

	//_DBG(LOG_TAG" stopClient() step out");
}

int CClientMeetingAgent::onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody)
{
	//_DBG(LOG_TAG" onResponse() step in");

	switch ((unsigned int)nCommand)
	{
	case enquire_link_response:
		_log(LOG_TAG_COLORED" onResponse() enquire_link_response");
		enquireLinkYo->zeroBalance();
		break;
	case bind_response:
		_log(LOG_TAG_COLORED" onResponse() bind_response; bind ok, start EnquireLinkYo");
		enquireLinkYo->start();
		break;
	case unbind_response:
		_log(LOG_TAG_COLORED" onResponse() unbind_response");
		break;
	default:
		_log(LOG_TAG" onResponse() unhandled nCommand %s", numberToHex(nCommand).c_str());
		break;
	}
	
	return TRUE;
}

int CClientMeetingAgent::onSmartBuildingQrCodeTokenRequest(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	//_DBG(LOG_TAG" onSmartBuildingQrCodeToken() step in");

	string strRequestBodyData = string(reinterpret_cast<const char *>(szBody));
	string strResponseBodyData = "";
	_log(LOG_TAG" onSmartBuildingQrCodeToken() body: %s", strRequestBodyData.c_str());

	JSONObject reqJson(strRequestBodyData);
	string reqUserId = reqJson.getString("USER_ID", "");
	string reqQrCodeToken = reqJson.getString("QRCODE_TOKEN", "");
	
	std::unique_ptr<JSONObject> decodedQrCodeJson(decodeQRCodeString(reqQrCodeToken));
	reqJson.release();

	if (reqQrCodeToken.size() < 1 || decodedQrCodeJson == nullptr || !decodedQrCodeJson->isValid())
	{
		response(getSocketfd(), nCommand, STATUS_ROK, nSequence, JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE);
		return TRUE;
	}

	string decStrQrCodeType = decodedQrCodeJson->getString("QRCODE_TYPE", "");
	std::regex qrCodeTypePattern(QR_CODE_TYPE_PATTERN);

	if (!regex_match(decStrQrCodeType, qrCodeTypePattern))
	{
		response(getSocketfd(), nCommand, STATUS_ROK, nSequence, JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE);
		return TRUE;
	}
	
	int decQrCodeType;
	convertFromString(decQrCodeType, decStrQrCodeType);

	if (decQrCodeType == 1)
	{
		// 開會通知
		if (reqUserId.compare("null") != 0)
		{
			strResponseBodyData = JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE;
		}
		else
		{
			// this type of QR code contains exactly the content to be sent back to user
			strResponseBodyData = decodedQrCodeJson->toUnformattedString();
		}
	}
	else if (decQrCodeType == 2)
	{
		// 數位簽到
		if (reqUserId.size() < 1)
		{
			strResponseBodyData = JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE;
		}
		else if (reqUserId.compare(TEST_USER_CAN_OPEN_101) == 0)
		{
			strResponseBodyData =
				R"({"QRCODE_TYPE":"2","MESSAGE":{"RESULT":true,"RESULT_MESSAGE":"Sign Up Successful"}})";
		}
		else
		{
			strResponseBodyData =
				R"({"QRCODE_TYPE":"2","MESSAGE":{"RESULT":false,"RESULT_MESSAGE":"You Have No Meeting Today"}})";
		}
	}
	else if (decQrCodeType == 3)
	{
		// 門禁
		string dstRoomId = decodedQrCodeJson->getString("DOOR_OPEN_ROOM_ID", "");

		if (reqUserId.size() < 1 || dstRoomId.size() < 1)
		{
			strResponseBodyData = JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE;
		}
		else
		{
			std::string resultMessage;
			bool handlerRet = doorAccessHandler.doRequest(resultMessage, reqUserId, dstRoomId);

			strResponseBodyData = "{\"QRCODE_TYPE\":\"3\",\"MESSAGE\":{\"RESULT\":"
				+ string(handlerRet ? "true" : "false") + ",\"RESULT_MESSAGE\":\"" + resultMessage + "\"}}";
		}
	}
	else
	{
		strResponseBodyData = JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE;
	}
	
	decodedQrCodeJson->release();
	response(getSocketfd(), nCommand, STATUS_ROK, nSequence, strResponseBodyData.c_str());
	return TRUE;
}

int CClientMeetingAgent::onSmartBuildingAppVersionRequest(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	//_DBG("[CClientMeetingAgent] onSmartBuildingAppVersion() step in");
	string bodyData;

	if (appVersionHandler == nullptr)
	{
		bodyData =
			R"({"VERSION": "0.0.0", "VERSION_CODE": 0, "APP_DOWNLOAD_URL": ""})";
	}
	else
	{
		bodyData =
			"{\"VERSION\": \"" + appVersionHandler->getVersionName()
			+ "\", \"VERSION_CODE\": " + std::to_string(appVersionHandler->getVersionCode())
			+ ", \"APP_DOWNLOAD_URL\": \"" + appVersionHandler->getDownloadLink() + "\"}";
	}

	response(getSocketfd(), nCommand, STATUS_ROK, nSequence, bodyData.c_str());
	return TRUE;
}

int CClientMeetingAgent::onSmartBuildingMeetingDataRequest(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	//_DBG("[CClientMeetingAgent] onSmartBuildingMeetingData() step in");
	
	string strRequestBodyData = string(reinterpret_cast<const char *>(szBody));
	string strResponseBodyData = "";
	_log("[ClientMeetingAgent] onSmartBuildingMeetingData() body: %s", strRequestBodyData.c_str());

	if (strRequestBodyData.find(TEST_USER_CAN_OPEN_101) != string::npos)
	{
		strResponseBodyData =
			"{\"USER_ID\":\"00000000-ffff-0000-ffff-ffffffffffff\",\"USER_NAME\":\"李二二\",\"USER_EMAIL\":\"qwwwew@gmail.com\",\"MEETING_DATA\":[{\"MEETING_ID\":\"a46595d0-fbcd-4d56-8bdc-3d8fa659b6a1\",\"SUPJECT\":\"XXX公司會議\",\"START_TIME\":\"2016-06-30 09:30:00\",\"END_TIME\":\"2016-06-30 12:30:00\",\"ROOM_ID\":\"ITES_101\",\"OWNER\":\"王一一\",\"OWNER_EMAIL\":\"qwer1234@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1afd78\",\"SUPJECT\":\"促進XXX發展計畫\",\"START_TIME\":\"2016-07-30 09:30:00\",\"END_TIME\":\"2016-07-30 12:30:00\",\"ROOM_ID\":\"ITES_102\",\"OWNER\":\"王一二\",\"OWNER_EMAIL\":\"qoiu1234@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass78\",\"SUPJECT\":\"nnnn公司會議\",\"START_TIME\":\"2016-08-30 09:30:00\",\"END_TIME\":\"2016-08-30 12:30:00\",\"ROOM_ID\":\"ITES_103\",\"OWNER\":\"王一三\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass71\",\"SUPJECT\":\"促進WWWW發展計畫\",\"START_TIME\":\"2016-08-31 09:30:00\",\"END_TIME\":\"2016-08-31 12:30:00\",\"ROOM_ID\":\"ITES_102\",\"OWNER\":\"王一四\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass72\",\"SUPJECT\":\"促進YYY發展計畫\",\"START_TIME\":\"2016-09-30 09:30:00\",\"END_TIME\":\"2016-09-30 12:30:00\",\"ROOM_ID\":\"ITES_103\",\"OWNER\":\"王一五\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46c0-b0c0-00eede1ass73\",\"SUPJECT\":\"XXXx公司會議\",\"START_TIME\":\"2016-10-30 09:30:00\",\"END_TIME\":\"2016-10-30 12:30:00\",\"ROOM_ID\":\"ITES_103\",\"OWNER\":\"王一六\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-43b0-b0c0-00eede1ass74\",\"SUPJECT\":\"促進YXXY發展計畫\",\"START_TIME\":\"2016-11-28 09:30:00\",\"END_TIME\":\"2016-11-28 12:30:00\",\"ROOM_ID\":\"ITES_104\",\"OWNER\":\"王一七\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass75\",\"SUPJECT\":\"促進WWXY發展計畫\",\"START_TIME\":\"2016-11-29 09:30:00\",\"END_TIME\":\"2016-11-29 12:30:00\",\"ROOM_ID\":\"ITES_104\",\"OWNER\":\"王一八\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass76\",\"SUPJECT\":\"促進WWY發展計畫\",\"START_TIME\":\"2016-11-30 09:30:00\",\"END_TIME\":\"2016-11-30 12:30:00\",\"ROOM_ID\":\"ITES_104\",\"OWNER\":\"王一九\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass18\",\"SUPJECT\":\"促進YYYX發展計畫\",\"START_TIME\":\"2016-12-30 09:30:00\",\"END_TIME\":\"2016-12-30 12:30:00\",\"ROOM_ID\":\"ITES_103\",\"OWNER\":\"王一十\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass28\",\"SUPJECT\":\"促進YYYW發展計畫\",\"START_TIME\":\"2016-12-31 09:30:00\",\"END_TIME\":\"2016-12-31 12:30:00\",\"ROOM_ID\":\"ITES_102\",\"OWNER\":\"王二一\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"}]}";

	}
	else if (strRequestBodyData.find(TEST_USER_CAN_OPEN_102) != string::npos)
	{
		strResponseBodyData =
			"{\"USER_ID\":\"ffffffff-ffff-0000-0000-ffffffffffff\",\"USER_NAME\":\"王二二\",\"USER_EMAIL\":\"qqdsdw@iii.org.tw\",\"MEETING_DATA\":[{\"MEETING_ID\":\"a46595d0-fbcd-4d56-8bdc-3d8fa659b6a1\",\"SUPJECT\":\"XXX公司會議\",\"START_TIME\":\"2017-06-30 09:30:00\",\"END_TIME\":\"2017-06-30 12:30:00\",\"ROOM_ID\":\"ITES_101\",\"OWNER\":\"王一一\",\"OWNER_EMAIL\":\"qwer1234@iii.org.tw\"},{\"MEETING_ID\":\"95999b7e-f56f-46b0-b0c0-00eede1ass78\",\"SUPJECT\":\"nnnn公司會議\",\"START_TIME\":\"2017-08-30 09:30:00\",\"END_TIME\":\"2017-08-30 12:30:00\",\"ROOM_ID\":\"ITES_103\",\"OWNER\":\"王一三\",\"OWNER_EMAIL\":\"qoiu1234222@iii.org.tw\"}]}";
	}
	else
	{
		strResponseBodyData = "{\"USER_ID\":\"UNKNOWN USER\"}";
	}

	response(getSocketfd(), nCommand, STATUS_ROK, nSequence, strResponseBodyData.c_str());

	return TRUE;
}

int CClientMeetingAgent::onSmartBuildingAMXControlAccessRequest(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	//_DBG(LOG_TAG" onSmartBuildingAMXControlAccess() step in");

	string strRequestBodyData = string(reinterpret_cast<const char *>(szBody));
	string strResponseBodyData = "";
	std::stringstream ss;
	
	_log(LOG_TAG" onSmartBuildingAMXControlAccess() body: %s", strRequestBodyData.c_str());

	JSONObject reqJson(strRequestBodyData);

	// ROOM_ID is a descriptive string like "ITES_101", not a magic number
	string reqRoomId = reqJson.getString("ROOM_ID", "");
	string reqUserId = reqJson.getString("USER_ID", "");

	if (!reqJson.isValid() || reqUserId.size() < 1 || reqRoomId.size() < 1)
	{
		/*response what?
		response(getSocketfd(), nCommand, STATUS_ROK, nSequence, JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE);
		return TRUE;*/
	}
	
	if (strRequestBodyData.find(TEST_USER_CAN_OPEN_101) != string::npos
		&& strRequestBodyData.find("ITES_101") != string::npos)
	{
		//OK can control AMX
		ss << "{\"USER_ID\": \"" << reqUserId
			<< "\", \"RESULT\": true, \"ROOM_IP\": \"" << amxControllerInfo->serverIp
			<< "\", \"ROOM_PORT\": " << amxControllerInfo->devicePort
			<< ", \"ROOM_TOKEN\": \"" << TEST_AMX_TOKEN << "\"}";
		strResponseBodyData = ss.str();
	}
	else
	{
		//NO cannot control AMX
		ss << "{\"USER_ID\": \"" << reqUserId << "\", \"RESULT\": false}";
		strResponseBodyData = ss.str();
	}

	response(getSocketfd(), nCommand, STATUS_ROK, nSequence, strResponseBodyData.c_str());
	return TRUE;
}

JSONObject *CClientMeetingAgent::decodeQRCodeString(std::string& src)
{
	if (src.size() < 1)
	{
		return nullptr;
	}

	if (src.compare(QR_CODE_MEETING_NOTICE_TEST_USER_CAN_OPEN_101) == 0)
	{
		// test user w/ AMX control permission
		return new JSONObject(R"({"QRCODE_TYPE": "1","MESSAGE":{"USER_ID": "00000000-ffff-0000-ffff-ffffffffffff"}})");
	}
	else if (src.compare(QR_CODE_MEETING_NOTICE_TEST_USER_CAN_OPEN_102) == 0)
	{
		// test user w/o AMX control permission
		return new JSONObject(R"({"QRCODE_TYPE": "1","MESSAGE":{"USER_ID": "ffffffff-ffff-0000-0000-ffffffffffff"}})");
	}
	else if (src.compare(QR_CODE_DIGITAL_CHECKIN_ITES) == 0)
	{
		// digital checkin
		return new JSONObject(R"({"DIGIT_SIGN_PLACE": "ITeS", "QRCODE_TYPE": "2"})");
	}
	else if (src.compare(QR_CODE_DOOR_101_DUMMY) == 0)
	{
		return new JSONObject(R"({"DOOR_OPEN_ROOM_ID": "ITES_101_DUMMY", "QRCODE_TYPE": "3"})");
	}
	else if (src.compare(QR_CODE_DOOR_102_DUMMY) == 0)
	{
		return new JSONObject(R"({"DOOR_OPEN_ROOM_ID": "ITES_102_DUMMY", "QRCODE_TYPE": "3"})");
	}
	else if (src.compare(QR_CODE_DOOR_101) == 0)
	{
		return new JSONObject(R"({"DOOR_OPEN_ROOM_ID": "ITES_101", "QRCODE_TYPE": "3"})");
	}
	else if (src.compare(QR_CODE_DOOR_102) == 0)
	{
		return new JSONObject(R"({"DOOR_OPEN_ROOM_ID": "ITES_102", "QRCODE_TYPE": "3"})");
	}

	return nullptr;
}

void CClientMeetingAgent::onServerDisconnect(unsigned long int nSocketFD)
{
	//_DBG(LOG_TAG" onServerDisconnect() step in");
	CCmpClient::onServerDisconnect(nSocketFD);

	// let CController decide what to do
	_log(LOG_TAG" Server actively disconnected");
	mpController->sendMessage(EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT,
		0, 0, NULL);
	
	//_DBG(LOG_TAG" onServerDisconnect() step out");
}

std::string CClientMeetingAgent::taskName()
{
	return TASK_NAME;
}
