#include "DoorAccessHandler.h"

#include "common.h"
#include "utility.h"
#include "../HiddenUtility.hpp"
#include "../RegexPattern.h"
#include "../TestStringsDefinition.h"
#include "CConfig.h"
#include "Ites1fDacClient.h"

#define LOG_TAG "[DoorAccessHandler]"
#define CONF_BLOCK_CLIENT_ITES_1F_CONTROLLER "CLIENT ITES 1F DOOR CONTROLLER"

DoorAccessHandler::DoorAccessHandler()
{
	meetingRoomToReaderMap["ITES_101"] = "001";
	meetingRoomToReaderMap["ITES_102"] = "002";
	meetingRoomToReaderMap["ITES_103"] = "003";
	meetingRoomToReaderMap["ITES_104"] = "004";
	meetingRoomToReaderMap["ITES_105"] = "005";
	meetingRoomToReaderMap["ITES_106"] = "006";
	meetingRoomToReaderMap["ITES_107"] = "007";
	meetingRoomToReaderMap["ITES_108"] = "008";
	meetingRoomToReaderMap["ITES_109"] = "009";
	meetingRoomToReaderMap["ITES_110"] = "010";
	meetingRoomToReaderMap["ITES_111"] = "011";
	meetingRoomToReaderMap["ITES_112"] = "012";
}

DoorAccessHandler::~DoorAccessHandler()
{
}

int DoorAccessHandler::initMember(std::unique_ptr<CConfig>& config)
{
	string strItes1fServerIp = config->getValue(CONF_BLOCK_CLIENT_ITES_1F_CONTROLLER, "server_ip");
	string strItes1fServerPort = config->getValue(CONF_BLOCK_CLIENT_ITES_1F_CONTROLLER, "port");
	string strItes1fServerAesKey = config->getValue(CONF_BLOCK_CLIENT_ITES_1F_CONTROLLER, "aes_key");

	if (strItes1fServerIp.empty()
		|| strItes1fServerPort.empty()
		|| strItes1fServerAesKey.empty())
	{
		_log(LOG_TAG" configMember() 404");
		return FALSE;
	}

	int ites1fServerPort;
	convertFromString(ites1fServerPort, strItes1fServerPort);

	this->ites1fDoor = std::make_unique<Ites1fDacClient>(strItes1fServerIp,
		ites1fServerPort, (uint8_t*)strItes1fServerAesKey.c_str());

	return TRUE;
}

bool DoorAccessHandler::doRequest(std::string& resultMessage, std::string const& uuid, std::string const& meetingRoom)
{
	if (!HiddenUtility::RegexMatch(uuid, UUID_PATTERN))
	{
		_log(LOG_TAG" doRequest() ID %s is not a valid UUID", uuid.c_str());
		resultMessage = "Invalid parameter";
		return false;
	}
	else if (!HiddenUtility::RegexMatch(meetingRoom, MEETING_ROOM_ID_PATTERN))
	{
		_log(LOG_TAG" doRequest() meeting room ID %s is not valid", meetingRoom.c_str());
		resultMessage = "Invalid parameter";
		return false;
	}

	int64_t unixTimeNow = HiddenUtility::unixTimeMilli();
	list<map<string, string>> listRet;
	string strSQL = "SELECT t.uuid, t.effective, t.expiry FROM door_access_token as t, user as u, meeting_room as m "
		"WHERE u.uuid = '" + uuid + "' AND m.room_id = '" + meetingRoom
		+ "' AND t.effective <= " + to_string(unixTimeNow) + " AND t.expiry >= " + to_string(unixTimeNow)
		+ " AND t.user_id = u.id AND t.meeting_room_id = m.id AND t.valid = 1 AND u.valid = 1 AND m.valid = 1;";

	bool bRet = HiddenUtility::selectFromDb(LOG_TAG" doRequest()", strSQL, listRet);
	if (!bRet)
	{
		resultMessage = "Permission denied";
		return false;
	}
	else if (listRet.size() > 1)
	{
		_log(LOG_TAG" doRequest() db returned more than 1 result?");
	}

	auto& retRow = *listRet.begin();
	auto& retToken = retRow["uuid"];
	auto& retValidFrom = retRow["effective"];
	auto& retGoodThrough = retRow["expiry"];

	// return now if token is identified as simulation only
	if (retToken.compare(DOOR_TOKEN_101_DUMMY) == 0
		|| retToken.compare(DOOR_TOKEN_102_DUMMY) == 0)
	{
		resultMessage = meetingRoom + " Door Opened";
		return true;
	}

	// 如果一定時間內已經成功開門，因為考慮到門不會馬上被關上，故直接返回成功，不再發送指令到遠端
	if (lastOpenedTime.find(meetingRoom) != lastOpenedTime.end()
		&& unixTimeNow - lastOpenedTime[meetingRoom] < 2000)
	{
		resultMessage = "Door just opened";
		return true;
	}

	string reader;
	if (meetingRoomToReaderMap.find(meetingRoom) != meetingRoomToReaderMap.end())
	{
		reader = meetingRoomToReaderMap[meetingRoom];
	}
	else
	{
		resultMessage = "Meeting room not found";
		return false;
	}

	int64_t tokenValidFrom, tokenGoodThrough;
	convertFromString(tokenValidFrom, retValidFrom);
	convertFromString(tokenGoodThrough, retGoodThrough);

	std::string errorDescription;
	bool apiCallOk = ites1fDoor->doorOpen(errorDescription, uuid, reader, retToken, tokenValidFrom, tokenGoodThrough);
	if (!apiCallOk)
	{
		resultMessage = "Failed: " + errorDescription;
		return false;
	}
	else
	{
		lastOpenedTime[meetingRoom] = HiddenUtility::unixTimeMilli();
		resultMessage = "Opened";
		return true;
	}
}
