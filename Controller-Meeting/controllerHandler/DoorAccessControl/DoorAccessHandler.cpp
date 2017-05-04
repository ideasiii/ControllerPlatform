#include "DoorAccessHandler.h"
#include <chrono>
#include "DoorAccessControllerParams.h"
#include "../TestStringsDefinition.h"

DoorAccessHandler::DoorAccessHandler() :
		ites1fDoor((char *)ITES_1F_DOOR_ACCESS_CONTROLLER_IP, ITES_1F_DOOR_ACCESS_CONTROLLER_PORT)
{
}

DoorAccessHandler::~DoorAccessHandler()
{
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
		return "{\"QRCODE_TYPE\": \"3\",\"MESSAGE\":{\"RESULT\":false,\"RESULT_MESSAGE\": \"Bad timing to open " + meetingRoom + ""\"}}";
	}
	else
	{
		// 如果一定時間內已經成功開門，因為考慮到門不會馬上被關上，故直接返回成功，不再發送指令到遠端
		if (lastSuccessOpenedTime.find(reader) != lastSuccessOpenedTime.end()
			&& unixTimeNow - lastSuccessOpenedTime[reader] < 2000)
		{
			return "{\"QRCODE_TYPE\": \"3\",\"MESSAGE\":{\"RESULT\":true,\"RESULT_MESSAGE\": \"Door REALLY Opened (cached)\"}}";
		}

		std::string errorDescription;
		bool apiCallOk = ites1fDoor.doorOpen(errorDescription, uuid, reader, token, validFrom, goodThru);
		if (!apiCallOk)
		{
			return "{\"QRCODE_TYPE\": \"3\",\"MESSAGE\":{\"RESULT\":true,\"RESULT_MESSAGE\": \"Door REALLY failed to open: "
				+ errorDescription + "\"}}";
		}
		else
		{
			lastSuccessOpenedTime[reader] = unixTimeMilli();
			return "{\"QRCODE_TYPE\": \"3\",\"MESSAGE\":{\"RESULT\":true,\"RESULT_MESSAGE\": \"Door REALLY Opened\"}}";
		}		
	}
}

int64_t DoorAccessHandler::unixTimeMilli()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();
}
