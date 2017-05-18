#pragma once

#include <memory>
#include "CConfig.h"

class AppVersionHandler;

class AppVersionHandlerFactory
{
public:
	// 根據 config 檔有甚麼可以用的就初始化其中一種 AppVersionHandler
	static AppVersionHandler* createFromConfig(std::unique_ptr<CConfig> &config);

private:
	explicit AppVersionHandlerFactory();
	~AppVersionHandlerFactory();
};
