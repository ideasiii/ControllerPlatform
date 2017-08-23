#include "CClientMeetingAgent.h"

#include <cryptopp/base64.h>
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
#include "HiddenUtility.hpp"
#include "JSONArray.h"
#include "JSONObject.h"
#include "RegexPattern.h"
#include "TestStringsDefinition.h"

#define LOG_TAG "[CClientMeetingAgent]"
#define LOG_TAG_COLORED "[\033[1;31mCClientMeetingAgent\033[0m]"

#define TASK_NAME "ClientAgent"
#define CONF_BLOCK_MEETING_AGENT_CLIENT "CLIENT MEETING_AGENT"
#define JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE R"({"QRCODE_TYPE":"0","MESSAGE":{"RESULT_MESSAGE":"Unknown type of QR-Code"}})"

using namespace std;

CClientMeetingAgent::CClientMeetingAgent(CObject *controller) :
	mpController(controller), enquireLinkYo(new EnquireLinkYo("ClientAgent.ely", this,
		EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT, mpController))
{
}

CClientMeetingAgent::~CClientMeetingAgent()
{
	stopClient();
}

int CClientMeetingAgent::initMember(unique_ptr<CConfig>& config)
{
	if (initMeetingAgentServerParams(config) == FALSE)
	{
		return FALSE;
	}

	auto appVerHandlerRet = AppVersionHandlerFactory::createFromConfig(config);
	if (appVerHandlerRet == nullptr)
	{
		_log(LOG_TAG" onInitial(): Cannot spawn AppVersionHandler");
		return FALSE;
	}

	auto amxControllerInfoRet = CClientAmxControllerFactory::getServerInfoFromConfig(config);
	if (amxControllerInfoRet == nullptr)
	{
		_log(LOG_TAG" onInitial(): Cannot spawn AmxControllerInfo");
		return FALSE;
	}

	appVersionHandler.reset(appVerHandlerRet);
	amxControllerInfo.reset(amxControllerInfoRet);

	return doorAccessHandler.initMember(config);
}

int CClientMeetingAgent::initMeetingAgentServerParams(unique_ptr<CConfig> &config)
{
	string strServerIp = config->getValue(CONF_BLOCK_MEETING_AGENT_CLIENT, "server_ip");
	string strPort = config->getValue(CONF_BLOCK_MEETING_AGENT_CLIENT, "port");
	string strQrcodeKey = config->getValue(CONF_BLOCK_MEETING_AGENT_CLIENT, "qrcode_aes_key");

	if (strServerIp.empty() || strPort.empty() || strQrcodeKey.empty())
	{
		_log(LOG_TAG" initMeetingAgentServerParams() 404");
		return FALSE;
	}

	int nPort;
	convertFromString(nPort, strPort);
	agentIp = strServerIp;
	agentPort = nPort;

	HiddenUtility::sha256(qrCodeKey, strQrcodeKey);
	//_log(LOG_TAG" initMeetingAgentServerParams() plain qrCodeKey = %s", strQrcodeKey.c_str());
	//auto hexQrCodeKey = HiddenUtility::getHexString(qrCodeKey, AesCrypto::KeyLength);
	//_log(LOG_TAG" initMeetingAgentServerParams() hashed qrCodeKey = %s", hexQrCodeKey.c_str());

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
		_log(LOG_TAG" startClient() Send binding request to agent FAILED");
		return FALSE;
	}

	appVersionHandler->start();
	// enquireLinkYo starts in onResponse(), when binding response is received

	return TRUE;
}

void CClientMeetingAgent::stopClient()
{
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
		_log(LOG_TAG" stopClient() socket fd invalid, quit");
		return;
	}

	// server don't response to unbind_request
	// receiving unbind_response while destructing CCmpClient may cause segmentation fault
	request(getSocketfd(), unbind_request, STATUS_ROK, getSequence(), NULL);
	stop();
}

int CClientMeetingAgent::onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody)
{
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
	const char *charRequestBodyData = reinterpret_cast<const char *>(szBody);
	_log(LOG_TAG" onSmartBuildingQrCodeToken() body: %s", charRequestBodyData);

	JSONObject reqJson(charRequestBodyData);
	string reqUserId = reqJson.getString("USER_ID", "");
	string reqQrCodeToken = reqJson.getString("QRCODE_TOKEN", "");
	reqJson.release();

	string strResponseBodyData = handleQrCodeToken(reqUserId, reqQrCodeToken);
	response(getSocketfd(), nCommand, STATUS_ROK, nSequence, strResponseBodyData.c_str());

	return TRUE;
}

int CClientMeetingAgent::onSmartBuildingAppVersionRequest(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	string resp;

	if (appVersionHandler == nullptr)
	{
		resp = R"({"VERSION":"0.0.0","VERSION_CODE":0,"APP_DOWNLOAD_URL":""})";
	}
	else
	{
		resp = "{\"VERSION\":\"" + appVersionHandler->getVersionName()
			+ "\",\"VERSION_CODE\":" + std::to_string(appVersionHandler->getVersionCode())
			+ ",\"APP_DOWNLOAD_URL\":\"" + appVersionHandler->getDownloadLink() + "\"}";
	}

	response(getSocketfd(), nCommand, STATUS_ROK, nSequence, resp.c_str());
	return TRUE;
}

int CClientMeetingAgent::onSmartBuildingMeetingDataRequest(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	const char *charRequestBodyData = reinterpret_cast<const char *>(szBody);
	_log(LOG_TAG" onSmartBuildingMeetingDataRequest() body: %s", charRequestBodyData);

	JSONObject reqJson(charRequestBodyData);
	string reqUserId = reqJson.getString("USER_ID", "");
	reqJson.release();

	string strResp = getMeetingsInfo(reqUserId);
	response(getSocketfd(), nCommand, STATUS_ROK, nSequence, strResp.c_str());

	return TRUE;
}

int CClientMeetingAgent::onSmartBuildingAMXControlAccessRequest(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	const char *charRequestBodyData = reinterpret_cast<const char *>(szBody);
	_log(LOG_TAG" onSmartBuildingAMXControlAccess() body: %s", charRequestBodyData);

	// ROOM_ID is a descriptive string like "ITES_101" instead of a magic number
	JSONObject reqJson(charRequestBodyData);
	string reqRoomId = reqJson.getString("ROOM_ID", "");
	string reqUserId = reqJson.getString("USER_ID", "");
	string ret = getAMXControlToken(reqUserId, reqRoomId);
	reqJson.release();

	response(getSocketfd(), nCommand, STATUS_ROK, nSequence, ret.c_str());
	return TRUE;
}

string CClientMeetingAgent::handleQrCodeToken(const string& reqUserId, const string& reqQrCodeToken)
{
	//_log(LOG_TAG"handleQrCodeToken() reqUserId = `%s`, reqQrCodeToken = `%s`",
	//	reqUserId.c_str(), reqQrCodeToken.c_str());

	std::unique_ptr<JSONObject> decodedQrCodeJson = decodeQRCodeString(reqQrCodeToken);

	if (decodedQrCodeJson == nullptr || !decodedQrCodeJson->isValid())
	{
		return JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE;
	}

	string decStrQrCodeType = decodedQrCodeJson->getString("QRCODE_TYPE", "");
	_log(LOG_TAG"handleQrCodeToken() decStrQrCodeType = `%s`", decStrQrCodeType.c_str());

	if (!HiddenUtility::RegexMatch(decStrQrCodeType, QR_CODE_TYPE_PATTERN))
	{
		decodedQrCodeJson->release();
		return JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE;
	}

	string strResponseBodyData;
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
		strResponseBodyData = doDigitalSignup(reqUserId);
	}
	else if (decQrCodeType == 3)
	{
		// 門禁
		string dstRoomId = decodedQrCodeJson->getString("DOOR_OPEN_ROOM_ID", "");
		string resultMessage;
		bool handlerRet = doorAccessHandler.doRequest(resultMessage, reqUserId, dstRoomId);

		strResponseBodyData = "{\"QRCODE_TYPE\":\"3\",\"MESSAGE\":{\"RESULT\":"
			+ string(handlerRet ? "true" : "false") + ",\"RESULT_MESSAGE\":\"" + resultMessage + "\"}}";
	}
	else
	{
		strResponseBodyData = JSON_RESP_UNKNOWN_TYPE_OF_QR_CODE;
	}

	decodedQrCodeJson->release();
	return strResponseBodyData;
}

string CClientMeetingAgent::getMeetingsInfo(const string& userId)
{
	if (!HiddenUtility::RegexMatch(userId, UUID_PATTERN))
	{
		return R"({"USER_ID":")" + userId + R"(","MEETING_DATA":[],"_STATUS_MSG":"Invalid user"})";
	}

	list<map<string, string>> listRet;
	string strSQL = "SELECT `u`.`id`, `u`.`uuid`, `u`.`name`, `u`.`email` FROM `user` as `u` WHERE `u`.`uuid` = '"
		+ userId + "' AND `u`.`valid` = '1' LIMIT 1                                                    ";

	bool bRet = HiddenUtility::selectFromDb(LOG_TAG" getMeetingsInfo()", strSQL, listRet);
	if (!bRet)
	{
		_log(LOG_TAG" getMeetingsInfo() ID %s not registered", userId.c_str());
		return R"({"USER_ID":")" + userId + R"(","MEETING_DATA":[],"_STATUS_MSG":"Not registered"})";
	}

	JSONArray meetingDataJsonArr;
	JSONObject respJson;
	meetingDataJsonArr.create();
	respJson.create();

	auto& retRow = *listRet.begin();
	string strUserDbId = retRow["id"], respStr;
	respJson.put("USER_ID", retRow["uuid"]);
	respJson.put("USER_NAME", retRow["name"]);
	respJson.put("USER_EMAIL", retRow["email"]);

	listRet.clear();
	strSQL = "SELECT meetings_candidate.* FROM meeting_members AS mm, "
		"(SELECT mi.id AS mi_id, mi.uuid AS meeting_id, mi.subject AS supject, "
		"FROM_UNIXTIME(mi.time_start/1000, '%Y-%m-%d %H:%i') AS start_time, "
		"FROM_UNIXTIME(mi.time_end/1000, '%Y-%m-%d %H:%i') AS end_time, "
		"mr.room_id AS room_id, u.name as owner, u.email AS owner_email "
		"FROM meeting_members AS mm, user AS u, meeting_info AS mi, meeting_room AS mr "
		"WHERE mm.user_id = u.id AND mm.meeting_info_id = mi.id AND mi.room_id = mr.id";

	/*if (userId.compare(TEST_USER_CAN_OPEN_101) == 0 || userId.compare(TEST_USER_CAN_OPEN_101) == 0)
	{
		// 測試 user 挑出所有會議
	}
	else
	{
		// 挑出 user 於今天明天後天進行的會議
		strSQL += " AND mi.time_start >= (UNIX_TIMESTAMP(CURDATE()) * 1000) "
			"AND mi.time_start < (UNIX_TIMESTAMP(CURDATE() + INTERVAL 3 DAY) * 1000)";
	}*/

	strSQL += " AND mm.valid = 1 AND u.valid = 1 AND mi.valid = 1 AND mr.valid = 1"
		") AS meetings_candidate WHERE mm.user_id = '"
		+ strUserDbId + "' AND mm.meeting_info_id = meetings_candidate.mi_id AND mm.valid = 1";

	bRet = HiddenUtility::selectFromDb(LOG_TAG" getMeetingsInfo()", strSQL, listRet);
	if (!bRet)
	{
		_log(LOG_TAG" getMeetingsInfo() ID %s have no upcoming meetings", userId.c_str());

		respJson.put("MEETING_DATA", meetingDataJsonArr);
		meetingDataJsonArr.release();
		respStr = respJson.toUnformattedString();
		respJson.release();

		return respStr;
	}

	for (auto iter = listRet.begin(); iter != listRet.end(); iter++)
	{
		auto &retRow = *iter;
		bool hasDuplicate = false;

		// 因為同一個人在一場會議內可能同時為租借人 + 主持人 + 與會人員，
		// 所以得過濾重複的會議
		// 且因為 AMX 控制權不包含在此清單中，所以可以簡單地忽略重複的 meeting_id
		// 若之後此清單必須同時回傳 AMX 控制權、角色等參數時，找 hasDuplicate 的迴圈就不適用
		for (auto iterFindDup = listRet.begin(); *iterFindDup != *iter; iterFindDup++)
		{
			auto &retRow2 = *iterFindDup;
			if (retRow["meeting_id"].compare(retRow2["meeting_id"]) == 0)
			{
				hasDuplicate = true;
				break;
			}
		}

		if (hasDuplicate)
		{
			// 若有重複的 meeting_id，只添加清單內最先出現的會議資訊
			continue;
		}

		JSONObject meetingDataJsonObj;
		meetingDataJsonObj.create();

		meetingDataJsonObj.put("MEETING_ID", retRow["meeting_id"]);
		meetingDataJsonObj.put("SUPJECT", retRow["supject"]);
		meetingDataJsonObj.put("START_TIME", retRow["start_time"]);
		meetingDataJsonObj.put("END_TIME", retRow["end_time"]);
		meetingDataJsonObj.put("ROOM_ID", retRow["room_id"]);
		meetingDataJsonObj.put("OWNER", retRow["owner"]);
		meetingDataJsonObj.put("OWNER_EMAIL", retRow["owner_email"]);

		meetingDataJsonArr.add(meetingDataJsonObj);
		meetingDataJsonObj.release();
	}

	respJson.put("MEETING_DATA", meetingDataJsonArr);
	meetingDataJsonArr.release();
	respStr = respJson.toUnformattedString();
	respJson.release();

	return respStr;
}

string CClientMeetingAgent::doDigitalSignup(const string& userId)
{
	if (!HiddenUtility::RegexMatch(userId, UUID_PATTERN))
	{
		_log(LOG_TAG" doDigitalSignup() ID `%s` is not a valid UUID", userId.c_str());
		return R"({"QRCODE_TYPE":"2","MESSAGE":{"RESULT":false,"RESULT_MESSAGE":"Unknown user"}})";
	}

	list<map<string, string>> listRet;
	string strSQL = "SELECT `u`.`id` FROM `user` as `u` WHERE `u`.`uuid` = '"
		+ userId + "' AND `u`.`valid` = '1' LIMIT 1                                   ";

	bool bRet = HiddenUtility::selectFromDb(LOG_TAG" doDigitalSignup()", strSQL, listRet);
	if (!bRet)
	{
		_log(LOG_TAG" doDigitalSignup() ID `%s` not registered", userId.c_str());
		return R"({"QRCODE_TYPE":"2","MESSAGE":{"RESULT":false,"RESULT_MESSAGE":"Unknown user"}})";
	}

	JSONObject respJson, respMessageJson;
	respJson.create();
	respMessageJson.create();
	respJson.put("QRCODE_TYPE", "2");

	auto& retRow = *listRet.begin();
	string strUserDbId = retRow["id"], respStr;
	listRet.clear();

	// check if user has already signed today
	strSQL = "SELECT id FROM signin_record WHERE user_id = " + strUserDbId + " "
		"AND success = 1 AND time_when >= (UNIX_TIMESTAMP(CURDATE()) * 1000)           "
		"AND time_when < (UNIX_TIMESTAMP(CURDATE() + INTERVAL 1 DAY) * 1000)           ";

	bRet = HiddenUtility::selectFromDb(LOG_TAG" doDigitalSignup()", strSQL, listRet);
	if (bRet)
	{
		respMessageJson.put("RESULT", false);
		respMessageJson.put("RESULT_MESSAGE", "You have already signed up today");
		respJson.put("MESSAGE", respMessageJson);

		respMessageJson.release();
		respStr = respJson.toUnformattedString();
		respJson.release();
		return respStr;
	}

	listRet.clear();
	strSQL = "SELECT s.id, i.subject, i.time_start, i.time_end FROM attendance_sheet as s, "
		"meeting_members as m, meeting_info as i "
		"WHERE m.user_id = " + strUserDbId + " AND m.meeting_info_id = i.id "
		"AND s.meeting_members_id = m.id ";

	if (userId.compare(TEST_USER_CAN_OPEN_101) == 0)
	{
		// 從簽到表挑出測試 user 所屬的會議名稱，不管會議是否在今天開始
		// do nothing here
	}
	else
	{
		// 從簽到表挑出 user 今天參加的第一場會議的名稱
		strSQL += "AND s.time_meeting_start >= (UNIX_TIMESTAMP(CURDATE()) * 1000) "
			"AND s.time_meeting_start < (UNIX_TIMESTAMP(CURDATE() + INTERVAL 1 DAY) * 1000) ";
	}

	strSQL += " AND s.valid = 1 AND m.valid = 1 AND i.valid = 1 ORDER BY i.time_start ASC LIMIT 1";

	bRet = HiddenUtility::selectFromDb(LOG_TAG" doDigitalSignup()", strSQL, listRet);
	string strSheetId;

	if (!bRet)
	{
		respMessageJson.put("RESULT", false);
		respMessageJson.put("RESULT_MESSAGE", "You have no meeting today");
		respJson.put("MESSAGE", respMessageJson);
		strSheetId = "-1";
	}
	else
	{
		auto& retRow = *listRet.begin();
		respMessageJson.put("RESULT", true);
		respMessageJson.put("RESULT_MESSAGE", "Your first meeting today: " + retRow["subject"]);
		respJson.put("MESSAGE", respMessageJson);
		strSheetId = retRow["id"];
	}

	// put record
	strSQL = "INSERT INTO `signin_record` (`user_id`, `success`, `attendance_sheet_id`, `time_when`) VALUES ('"
		+ strUserDbId + "', '" + (bRet ? "1" : "0") + "', '" + strSheetId  + "', '"
		+ to_string(HiddenUtility::unixTimeMilli()) + "')                                   "
		"                                                                                   ";
	HiddenUtility::execOnDb(LOG_TAG" doDigitalSignup()", strSQL);

	respMessageJson.release();
	respStr = respJson.toUnformattedString();
	respJson.release();
	return respStr;
}

string CClientMeetingAgent::getAMXControlToken(const string& userId, const string& roomId)
{
	if (!HiddenUtility::RegexMatch(userId, UUID_PATTERN)
		|| !HiddenUtility::RegexMatch(roomId, MEETING_ROOM_ID_PATTERN))
	{
		_log(LOG_TAG" getAMXControlToken() user ID `%s` or room ID `%s` is not valid", userId.c_str(), roomId.c_str());
		return R"({"USER_ID":")" + userId + R"(","RESULT":false,"ROOM_IP":"",ROOM_PORT:-1,"ROOM_TOKEN":""})";
	}

	int64_t unixTimeNow = HiddenUtility::unixTimeMilli();
	list<map<string, string>> listRet;
	string strSQL = "SELECT token FROM amx_control_token as t, user as u, meeting_room as m"
		" WHERE u.uuid = '" + userId + "' AND m.room_id = '" + roomId + "'"
		+ " AND t.time_start <= " + to_string(unixTimeNow) + " AND t.time_end >= " + to_string(unixTimeNow)
		+ " AND t.user_id = u.id AND t.meeting_room_id = m.id AND t.valid = 1 AND u.valid = 1 AND m.valid = 1";

	bool bRet = HiddenUtility::selectFromDb(LOG_TAG" getAMXControlToken()", strSQL, listRet);
	if (!bRet)
	{
		return R"({"USER_ID":")" + userId + R"(","RESULT":false,"ROOM_IP":"",ROOM_PORT:-1,"ROOM_TOKEN":""})";
	}

	auto& retRow = *listRet.begin();
	auto& retToken = retRow["token"];

	return "{\"USER_ID\":\"" + userId
		+ "\",\"RESULT\":true,\"ROOM_IP\":\"" + amxControllerInfo->serverIp
		+ "\",\"ROOM_PORT\":" + to_string(amxControllerInfo->devicePort)
		+ ",\"ROOM_TOKEN\":\"" + retToken + "\"}";
}

unique_ptr<JSONObject> CClientMeetingAgent::decodeQRCodeString(const string& src) const
{
	if (src.size() < 1)
	{
		return nullptr;
	}
	else if (src.compare(QR_CODE_MEETING_NOTICE_TEST_USER_CAN_OPEN_101) == 0)
	{
		// test user w/ AMX control permission
		return make_unique<JSONObject>(R"({"QRCODE_TYPE": "1","MESSAGE":{"USER_ID": "00000000-ffff-0000-ffff-ffffffffffff"}})");
	}
	else if (src.compare(QR_CODE_MEETING_NOTICE_TEST_USER_CAN_OPEN_102) == 0)
	{
		// test user w/o AMX control permission
		return make_unique<JSONObject>(R"({"QRCODE_TYPE": "1","MESSAGE":{"USER_ID": "ffffffff-ffff-0000-0000-ffffffffffff"}})");
	}
	else if (src.compare(QR_CODE_DIGITAL_CHECKIN_ITES) == 0)
	{
		// test digital checkin
		return make_unique<JSONObject>(R"({"DIGIT_SIGN_PLACE": "ITeS", "QRCODE_TYPE": "2"})");
	}
	else if (src.compare(QR_CODE_DOOR_101_DUMMY) == 0)
	{
		// dry run for room 101 door open
		return make_unique<JSONObject>(R"({"DOOR_OPEN_ROOM_ID": "ITES_101_DUMMY", "QRCODE_TYPE": "3"})");
	}
	else if (src.compare(QR_CODE_DOOR_102_DUMMY) == 0)
	{
		// dry run for room 102 door open
		return make_unique<JSONObject>(R"({"DOOR_OPEN_ROOM_ID": "ITES_102_DUMMY", "QRCODE_TYPE": "3"})");
	}
	else if (src.compare(QR_CODE_DOOR_101) == 0)
	{
		return make_unique<JSONObject>(R"({"DOOR_OPEN_ROOM_ID": "ITES_101", "QRCODE_TYPE": "3"})");
	}
	else if (src.compare(QR_CODE_DOOR_102) == 0)
	{
		return make_unique<JSONObject>(R"({"DOOR_OPEN_ROOM_ID": "ITES_102", "QRCODE_TYPE": "3"})");
	}

	return decryptQRCodeString(src);
}

unique_ptr<JSONObject> CClientMeetingAgent::decryptQRCodeString(const string& src) const
{
	std::vector<uint8_t> srcDecoded = HiddenUtility::decodeBase64(src);
	//printf("srcDecodedByte (%lu): ", srcDecoded.size());
    //HiddenUtility::arrayToOneLineHex(srcDecoded.data(), srcDecoded.size());

	if (srcDecoded.size() <= AesCrypto::IvLength)
	{
		return nullptr;
	}

    byte *iv = srcDecoded.data();
    byte *cipherRaw = srcDecoded.data() + AesCrypto::IvLength;
    uint cipherLen = srcDecoded.size() - AesCrypto::IvLength;
	//AesCrypto::splitIvAndCipher(srcDecoded.data(), srcDecoded.size(), &iv, &cipherRaw);

    unique_ptr<AesCrypto> crypto(AesCrypto::createCtrInstance(qrCodeKey));
    string decryptedText = crypto->decrypt(cipherRaw, cipherLen, iv);
	_log(LOG_TAG" decryptQRCodeString() decryptedText = %s", decryptedText.c_str());

	return make_unique<JSONObject>(decryptedText);
}

void CClientMeetingAgent::onServerDisconnect(unsigned long int nSocketFD)
{
	CCmpClient::onServerDisconnect(nSocketFD);

	// let CController decide what to do
	_log(LOG_TAG" Server actively disconnected");
	mpController->sendMessage(EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MEETING_AGENT,
		0, 0, NULL);
}

string CClientMeetingAgent::taskName()
{
	return TASK_NAME;
}
