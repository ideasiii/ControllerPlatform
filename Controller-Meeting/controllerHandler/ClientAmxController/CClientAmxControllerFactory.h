#pragma once

#include <memory>
#include "AmxControllerInfo.h"

class CClientAmxController;
class CConfig;
class CObject;

class CClientAmxControllerFactory
{
public:
	// 根據 config 檔有甚麼可以用的就初始化 CClientAmxController
	static CClientAmxController* createFromConfig(std::unique_ptr<CConfig> &config, CObject *controller);

	static AmxControllerInfo* getServerInfoFromConfig(std::unique_ptr<CConfig> &config);

private:
	explicit CClientAmxControllerFactory();
	~CClientAmxControllerFactory();
};
