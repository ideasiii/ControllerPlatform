#include "DoorAccessHandler.h"

#include <chrono>
#include "common.h"
#include "utility.h"
#include "../TestStringsDefinition.h"
#include "CConfig.h"

#define CONF_BLOCK_CLIENT_ITES_1F_CONTROLLER "CLIENT ITES 1F DOOR CONTROLLER"

DoorAccessHandler::DoorAccessHandler()
{
}

DoorAccessHandler::~DoorAccessHandler()
{
}

int DoorAccessHandler::initMember(std::unique_ptr<CConfig>& config)
{
	string strItes1fServerIp = config->getValue(CONF_BLOCK_CLIENT_ITES_1F_CONTROLLER, "server_ip");
	string strItes1fServerPort = config->getValue(CONF_BLOCK_CLIENT_ITES_1F_CONTROLLER, "port");
	string strItes1fServerAesKey = config->getValue(CONF_BLOCK_CLIENT_ITES_1F_CONTROLLER, "aes_key");

	if (strItes1fServerIp.empty() || strItes1fServerPort.empty()
		|| strItes1fServerAesKey.empty())
	{
		_log("[DoorAccessHandler] configMember() 404");
		return FALSE;
	}

	int ites1fServerPort;
	convertFromString(ites1fServerPort, strItes1fServerPort);
	
	this->ites1fDoor = std::make_unique<Ites1fDacClient>(strItes1fServerIp,
		ites1fServerPort, (uint8_t*)strItes1fServerAesKey.c_str());

	return TRUE;
}

std::string DoorAccessHandler::doRequestDummy(std::string uuid, std::string meetingRoom)
{
	if (uuid.find(TEST_USER_HAS_MEETING_IN_001) != string::npos
		&& meetingRoom.find("101") != string::npos)
	{
		//101 door lock (dummy response)
		return "{\"QRCODE_TYPE\": \"3\",\"MESSAGE\":{\"RESULT\":true,\"RESULT_MESSAGE\": \"101 Door Opened (dummy)\"}}";
	}
	else if (uuid.find(TEST_USER_HAS_MEETING_IN_002) != string::npos
		&& meetingRoom.find("102") != string::npos)
	{
		//102 door lock (dummy response)
		return "{\"QRCODE_TYPE\": \"3\",\"MESSAGE\":{\"RESULT\":true,\"RESULT_MESSAGE\": \"102 Door Opened (dummy)\"}}";
	}

	return "{\"QRCODE_TYPE\":\"3\",\"MESSAGE\":{\"RESULT\":false,\"RESULT_MESSAGE\":\"Permission to " + meetingRoom + " is not granted\"}}";
}

/**
 * This function REALLY SENDS commannd to Controller-DoorControl
 */
std::string DoorAccessHandler::doRequest(std::string uuid, std::string meetingRoom)
{
	string token;
	string reader;

	if (uuid.find(TEST_USER_HAS_MEETING_IN_001) != string::npos
		&& meetingRoom.find("101") != string::npos)
	{
		// 101 door lock (REALLY opens the door)
		token = "eeeeeeee-ffff-0101-ffff-ffffffffffff";
		reader = "001";
	}
	else if (uuid.find(TEST_USER_HAS_MEETING_IN_002) != string::npos
		&& meetingRoom.find("102") != string::npos)
	{
		// 102 door lock (REALLY opens the door)		
		token = "eeeeeeee-ffff-0102-ffff-ffffffffffff";
		reader = "002";
	} 
	else
	{
		return "{\"QRCODE_TYPE\":\"3\",\"MESSAGE\":{\"RESULT\":false,\"RESULT_MESSAGE\":\"Permission to " + meetingRoom + " is not granted \"}}";
	}

	// TODO validate token
	int64_t extendTime = 100000;
	int64_t validFrom = unixTimeMilli() - extendTime;
	int64_t goodThru = unixTimeMilli() + extendTime;
	int64_t unixTimeNow = unixTimeMilli();

	if (unixTimeNow < validFrom || unixTimeNow > goodThru)
	{
		return "{\"QRCODE_TYPE\": \"3\",\"MESSAGE\":{\"RESULT\":false,\"RESULT_MESSAGE\": \"Bad timing to open " + meetingRoom + "\"}}";
	}

	// 如果一定時間內已經成功開門，因為考慮到門不會馬上被關上，故直接返回成功，不再發送指令到遠端
	if (lastOpenedTime.find(reader) != lastOpenedTime.end()
		&& unixTimeNow - lastOpenedTime[reader] < 2000)
	{
		return "{\"QRCODE_TYPE\": \"3\",\"MESSAGE\":{\"RESULT\":true,\"RESULT_MESSAGE\": \"Door REALLY Opened (cached)\"}}";
	}

	std::string errorDescription;
	bool apiCallOk = ites1fDoor->doorOpen(errorDescription, uuid, reader, token, validFrom, goodThru);
	if (!apiCallOk)
	{
		return "{\"QRCODE_TYPE\": \"3\",\"MESSAGE\":{\"RESULT\":true,\"RESULT_MESSAGE\": \"Door REALLY failed to open: "
			+ errorDescription + "\"}}";
	}
	else
	{
		lastOpenedTime[reader] = unixTimeMilli();
		return "{\"QRCODE_TYPE\": \"3\",\"MESSAGE\":{\"RESULT\":true,\"RESULT_MESSAGE\": \"Door REALLY Opened\"}}";
	}		
	
}

int64_t DoorAccessHandler::unixTimeMilli()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();
}
