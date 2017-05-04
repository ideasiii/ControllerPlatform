#pragma once

#include <string>
#include <map>
#include "Ites1fDacClient.h"

/**
 * 處理門禁相關事務
 */
class DoorAccessHandler
{
public:
	DoorAccessHandler();
	~DoorAccessHandler();
	
	std::string doRequestDummy(std::string uuid, std::string meetingRoom);
	std::string doRequest(std::string uuid, std::string meetingRoom);

private:
	Ites1fDacClient ites1fDoor;

	/**
	 * returns Unix timestamp in milliseconds
	 */
	int64_t unixTimeMilli();

	/**
	 * 紀錄門最後成功被打開的時間
	 */
	std::map<string, int64_t> lastOpenedTime;
};