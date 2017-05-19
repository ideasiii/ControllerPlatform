#pragma once

#include <string>

class AmxControllerInfo
{
public:
	// 伺服器的 IP
	std::string serverIp;

	// 供裝置連接進行設備控制的連接埠
	int devicePort;

	// 供 ClientAmxController 連接進行 token 驗證的連接埠
	int validationPort;

	AmxControllerInfo(std::string serverIp, int devicePort, int validationPort);
};
