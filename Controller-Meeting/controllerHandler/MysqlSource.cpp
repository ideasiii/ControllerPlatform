#include "MysqlSource.h"

#include "common.h"
#include "utility.h"
#include "CConfig.h"
#include "CMysqlHandler.h"
#include "LogHandler.h"

#define LOG_TAG "MysqlSource"
#define CONF_BLOCK_MYSQL_SOURCE "MYSQL SOURCE"

using namespace std;

MysqlSource::MysqlSource() : initialized(false)
{
}

MysqlSource::~MysqlSource()
{
}

MysqlSource& MysqlSource::getInstance()
{
	static MysqlSource instance;
	return instance;
}

bool MysqlSource::initialize(std::unique_ptr<CConfig>& config)
{
	if (config == nullptr)
	{
		_log(LOG_TAG" initialize(): config is nullptr");
		return false;
	}
	else if (initialized)
	{
		_log(LOG_TAG" initialize(): already initialized");
		return false;
	}

	string strMysqlHost = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "host");
	string strMysqlPort = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "port");
	string strMysqlUser = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "user");
	string strMysqlPassword = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "password");
	string strMysqlDatabase = config->getValue(CONF_BLOCK_MYSQL_SOURCE, "database");

	if (strMysqlHost.empty()
		|| strMysqlPort.empty()
		|| strMysqlUser.empty()
		|| strMysqlPassword.empty()
		|| strMysqlDatabase.empty())
	{
		_log(LOG_TAG" initialize(): config 404");
		return false;
	}

	int devicePort;
	int validationPort;
	int mysqlPort;

	convertFromString(mysqlPort, strMysqlPort);

	initialized = true;
	host = strMysqlHost;
	port = mysqlPort;
	user = strMysqlUser;
	password = strMysqlPassword;
	database = strMysqlDatabase;

	return true;
}

CMysqlHandler* MysqlSource::getMysqlHandler()
{
	CMysqlHandler* mysql = new CMysqlHandler();
	int nRet = mysql->connect(host, database, user, password);

	if (FALSE == nRet)
	{
		_log(LOG_TAG" getMysqlHandler() Mysql Error: %s", mysql->getLastError().c_str());
		delete mysql;
		return nullptr;
	}

	return mysql;
}
