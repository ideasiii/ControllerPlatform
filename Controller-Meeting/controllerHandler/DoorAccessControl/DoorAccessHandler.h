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
	
	// 處理 (測試 QR code) 的要求
	std::string doRequestDummy(std::string uuid, std::string meetingRoom);

	// 處理 (真的 QR code) 的要求 
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