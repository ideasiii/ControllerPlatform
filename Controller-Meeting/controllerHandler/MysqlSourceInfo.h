#pragma once

#include <string>

class MysqlSourceInfo
{
public:
	MysqlSourceInfo(std::string host, int port, std::string user, std::string password, std::string database)
		: host(host), port(port), user(user), password(password), database(database)
	{
	}

	MysqlSourceInfo(const MysqlSourceInfo& src)
		: host(src.host), port(src.port), user(src.user), password(src.password), database(src.database)
	{
	}

	~MysqlSourceInfo()
	{
	}
	
	const std::string host;
	const int port;
	const std::string user;
	const std::string password;
	const std::string database;
};
