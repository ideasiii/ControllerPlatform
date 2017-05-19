#include "CClientAmxControllerFactory.h"

#include <string>
#include "utility.h"
#include "CClientAmxController.h"
#include "CConfig.h"
#include "LogHandler.h"

#define CONF_BLOCK_AMX_CONTROLLER "CLIENT AMX CONTROLLER"

using namespace std;

CClientAmxController* CClientAmxControllerFactory::createFromConfig(std::unique_ptr<CConfig> &config)
{
	string strControllerIp = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "server_ip");
	string strAmxControllerDevicePort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "user_port");
	string strAmxControllerValidationPort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "validation_port");
	if (strControllerIp.empty() || strAmxControllerDevicePort.empty()
		|| strAmxControllerValidationPort.empty())
	{
		_log("[CController] onInitial(): AMX controller client config 404");
		return nullptr;
	}

	int devicePort;
	int validationPort;
	convertFromString(devicePort, strAmxControllerDevicePort);
	convertFromString(validationPort, strAmxControllerValidationPort);

	_log("[CClientAmxControllerFactory] init CClientAmxController with config parameters");	
	return new CClientAmxController(strControllerIp,
		devicePort, validationPort);
}

AmxControllerInfo* CClientAmxControllerFactory::getServerInfoFromConfig(std::unique_ptr<CConfig>& config)
{
	string strControllerIp = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "server_ip");
	string strAmxControllerDevicePort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "user_port");
	string strAmxControllerValidationPort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "validation_port");
	if (strControllerIp.empty() || strAmxControllerDevicePort.empty()
		|| strAmxControllerValidationPort.empty())
	{
		_log("[CController] onInitial(): AMX controller client config 404");
		return nullptr;
	}

	int devicePort;
	int validationPort;
	convertFromString(devicePort, strAmxControllerDevicePort);
	convertFromString(validationPort, strAmxControllerValidationPort);

	return new AmxControllerInfo(strControllerIp, devicePort, validationPort);
}

CClientAmxControllerFactory::CClientAmxControllerFactory()
{
}

CClientAmxControllerFactory::~CClientAmxControllerFactory()
{
}
