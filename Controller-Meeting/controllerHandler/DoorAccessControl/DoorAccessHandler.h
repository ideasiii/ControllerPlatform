#pragma once

#include <map>
#include <memory>
#include <string>

#include "../MysqlSourceInfo.h"

class CConfig;
class Ites1fDacClient;

/**
 * 處理門禁相關事務
 */
class DoorAccessHandler
{
public:
	DoorAccessHandler();
	~DoorAccessHandler();
	
	// Intializes members that needs parameters in config.
	// Returns FALSE if anything bad happens
	int initMember(std::unique_ptr<CConfig> &config);
	
	// 處理開門要求
	// resultMessage: (output) 關於處理結果的描述訊息
	// uuid: (intput) 使用者的 UUID
	// meetingRoom: (input) 欲開啟的會議室 ID
	bool doRequest(std::string& resultMessage, std::string const& uuid, std::string const& meetingRoom);

private:
	std::unique_ptr<Ites1fDacClient> ites1fDoor;

	// 紀錄門最後成功被打開的時間
	std::map<std::string, int64_t> lastOpenedTime;
	std::map<std::string, std::string> meetingRoomToReaderMap;

	MysqlSourceInfo mysqlSourceInfo;
};
