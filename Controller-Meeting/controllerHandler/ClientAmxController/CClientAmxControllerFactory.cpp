#include "CClientAmxControllerFactory.h"

#include <string>
#include "CClientAmxController.h"
#include "CConfig.h"
#include "LogHandler.h"

#define CONF_BLOCK_AMX_CONTROLLER "CLIENT AMX CONTROLLER"

using namespace std;

CClientAmxController* CClientAmxControllerFactory::createFromConfig(std::unique_ptr<CConfig> &config)
{
	string strAmxControllerIp = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "server_ip");
	string strAmxControllerUserControlPort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "user_port");
	string strAmxControllerValidationPort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "validation_port");
	if (strAmxControllerIp.empty() || strAmxControllerUserControlPort.empty()
		|| strAmxControllerValidationPort.empty())
	{
		_log("[CController] onInitial(): AMX controller client config 404");
		return nullptr;
	}

	int amxControllerUserControlPort;
	int amxControllerValidationPort;
	convertFromString(amxControllerUserControlPort, strAmxControllerUserControlPort);
	convertFromString(amxControllerValidationPort, strAmxControllerValidationPort);

	_log("[CClientAmxControllerFactory] init CClientAmxController with config parameters");	
	return new CClientAmxController(strAmxControllerIp,
		amxControllerUserControlPort, amxControllerValidationPort);

}

CClientAmxControllerFactory::CClientAmxControllerFactory()
{
}

CClientAmxControllerFactory::~CClientAmxControllerFactory()
{
}
