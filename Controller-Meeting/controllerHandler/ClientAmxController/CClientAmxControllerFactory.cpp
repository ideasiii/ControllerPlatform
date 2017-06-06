#include "CClientAmxControllerFactory.h"

#include <string>
#include "../MysqlSourceInfo.h"
#include "utility.h"
#include "CClientAmxController.h"
#include "CConfig.h"
#include "CObject.h"
#include "LogHandler.h"

#define LOG_TAG "[CClientAmxControllerFactory]"
#define CONF_BLOCK_AMX_CONTROLLER "CLIENT AMX CONTROLLER"
#define CONF_BLOCK_MYSQL_SOURCE "MYSQL SOURCE"

using namespace std;

CClientAmxController* CClientAmxControllerFactory::createFromConfig(std::unique_ptr<CConfig> &config, CObject *controller)
{
	string strControllerIp = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "server_ip");
	string strAmxControllerDevicePort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "user_port");
	string strAmxControllerValidationPort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "validation_port");

	string strMysqlHost = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "host");
	string strMysqlPort = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "port");
	string strMysqlUser = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "user");
	string strMysqlPassword = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "password");
	string strMysqlDatabase = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "database");

	if (strControllerIp.empty() 
		|| strAmxControllerDevicePort.empty()
		|| strAmxControllerValidationPort.empty()
		|| strMysqlHost.empty()
		|| strMysqlPort.empty()
		|| strMysqlUser.empty()
		|| strMysqlPassword.empty()
		|| strMysqlDatabase.empty())
	{
		_log(LOG_TAG" createFromConfig(): AMX controller client config 404");
		return nullptr;
	}

	int devicePort;
	int validationPort;
	int mysqlPort;
	
	convertFromString(devicePort, strAmxControllerDevicePort);
	convertFromString(validationPort, strAmxControllerValidationPort);
	convertFromString(mysqlPort, strMysqlPort);

	MysqlSourceInfo mysqlSrc(strMysqlHost, mysqlPort, strMysqlUser, strMysqlPassword, strMysqlDatabase);
	
	_log(LOG_TAG" init CClientAmxController with config parameters");
	return new CClientAmxController(controller, strControllerIp,
		devicePort, validationPort, mysqlSrc);
}

AmxControllerInfo* CClientAmxControllerFactory::getServerInfoFromConfig(std::unique_ptr<CConfig>& config)
{
	string strControllerIp = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "server_ip");
	string strAmxControllerDevicePort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "user_port");
	string strAmxControllerValidationPort = config->getValue(CONF_BLOCK_AMX_CONTROLLER, "validation_port");
	if (strControllerIp.empty() || strAmxControllerDevicePort.empty()
		|| strAmxControllerValidationPort.empty())
	{
		_log(LOG_TAG" createFromConfig(): AMX controller client config 404");
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
