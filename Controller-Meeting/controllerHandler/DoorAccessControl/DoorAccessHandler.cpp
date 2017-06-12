#include "DoorAccessHandler.h"

#include <chrono>
#include <regex>

#include "common.h"
#include "utility.h"
#include "../HiddenUtility.hpp"
#include "../RegexPattern.h"
#include "../TestStringsDefinition.h"
#include "CConfig.h"
#include "CMysqlHandler.h"
#include "Ites1fDacClient.h"

#define LOG_TAG "[DoorAccessHandler]"
#define CONF_BLOCK_CLIENT_ITES_1F_CONTROLLER "CLIENT ITES 1F DOOR CONTROLLER"
#define CONF_BLOCK_MYSQL_SOURCE "MYSQL SOURCE"

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
	string strMysqlHost = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "host");
	string strMysqlPort = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "port");
	string strMysqlUser = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "user");
	string strMysqlPassword = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "password");
	string strMysqlDatabase = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "database");

	if (strItes1fServerIp.empty() 
		|| strItes1fServerPort.empty()
		|| strItes1fServerAesKey.empty()
		|| strMysqlHost.empty()
		|| strMysqlPort.empty()
		|| strMysqlUser.empty()
		|| strMysqlPassword.empty()
		|| strMysqlDatabase.empty())
	{
		_log(LOG_TAG" configMember() 404");
		return FALSE;
	}

	int ites1fServerPort;
	convertFromString(ites1fServerPort, strItes1fServerPort);
	
	this->ites1fDoor = std::make_unique<Ites1fDacClient>(strItes1fServerIp,
		ites1fServerPort, (uint8_t*)strItes1fServerAesKey.c_str());

	int mysqlPort;
	convertFromString(mysqlPort, strMysqlPort);
	mysqlSourceInfo = MysqlSourceInfo(strMysqlHost, mysqlPort, strMysqlUser, strMysqlPassword, strMysqlDatabase);

	return TRUE;
}

bool DoorAccessHandler::doRequest(std::string& resultMessage, std::string const& uuid, std::string const& meetingRoom)
{
	std::regex uuidRegex(UUID_PATTERN);
	std::regex meetingRoomHumanIdRegex(MEETING_ROOM_ID_PATTERN);

	if (!regex_match(uuid, uuidRegex))
	{
		_log(LOG_TAG" doRequest() ID %s is not a valid UUID", uuid.c_str());
		resultMessage = "Invalid parameter";
		return false;
	}
	else if (!regex_match(meetingRoom, meetingRoomHumanIdRegex))
	{
		_log(LOG_TAG" doRequest() meeting room ID %s is not valid", meetingRoom.c_str());
		resultMessage = "Invalid parameter";
		return false;
	}

	int64_t unixTimeNow = HiddenUtility::unixTimeMilli();
	CMysqlHandler mysql;
	int nRet = mysql.connect(mysqlSourceInfo.host, mysqlSourceInfo.database, mysqlSourceInfo.user,
		mysqlSourceInfo.password);

	if (FALSE == nRet)
	{
		_log(LOG_TAG" doRequest() Mysql Error: %s", mysql.getLastError().c_str());
		resultMessage = "System error";
		return false;
	}

	list<map<string, string> > listRet;
	string strSQL = "SELECT t.uuid, t.effective, t.expiry FROM meeting.door_access_token as t, meeting.user as u, meeting.meeting_room as m "
		"WHERE u.uuid = '" + uuid + "' AND m.room_id = '" + meetingRoom
		+ "' AND t.effective <= " + to_string(unixTimeNow) + " AND t.expiry >= " + to_string(unixTimeNow)
		+ " AND t.user_id = u.id AND t.meeting_room_id = m.id AND t.valid = 1 AND u.valid = 1 AND m.valid = 1;";

	nRet = mysql.query(strSQL, listRet);
	string strError = mysql.getLastError();
	mysql.close();

	if (FALSE == nRet)
	{
		_log(LOG_TAG" doRequest() Mysql Error: %s", strError.c_str());
		resultMessage = "System error";
		return false;
	}
	else if (listRet.size() < 1)
	{
		_log(LOG_TAG" doRequest() db no match token");
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
