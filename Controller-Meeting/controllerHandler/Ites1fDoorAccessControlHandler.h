#pragma once

#include <string>
#include "packet.h"

class Ites1fDoorAccessControlHandler
{
public:
	Ites1fDoorAccessControlHandler(char *ip, int port);
	~Ites1fDoorAccessControlHandler();

	/**
	 * 發送開門請求
	 * @param reader 要開啟的門的讀卡機編號
	 * @param validFrom token 生效時間, unix time in millisecond, UTC
	 * @param goodThrough token 有效的最後一刻，超過這個時間 token 就失效, unix time in millisecond, UTC
	 */
	bool doorOpen(std::string &errorDescription, std::string userUuid,
		std::string readerId, std::string token, int64_t validFrom, int64_t goodThrough);

private:
	char *serverIp;
	int serverPort;
	uint8_t *aesKey; 
};
