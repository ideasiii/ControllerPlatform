#pragma once

#include <string>

class MysqlSourceInfo
{
public:
	MysqlSourceInfo(std::string host, int port, std::string user, std::string password, std::string database)
		: host(host), port(port), user(user), password(password), database(database)
	{
	}
	
	MysqlSourceInfo()
		: host(""), port(0), user(""), password(""), database("")
	{
	}

	MysqlSourceInfo(const MysqlSourceInfo& src)
		: host(src.host), port(src.port), user(src.user), password(src.password), database(src.database)
	{
	}

	~MysqlSourceInfo()
	{
	}
	
	std::string host;
	int port;
	std::string user;
	std::string password;
	std::string database;
};
