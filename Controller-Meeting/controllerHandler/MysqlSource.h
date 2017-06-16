#pragma once

#include <memory>
#include <string>

class CMysqlHandler;
class CConfig;

class MysqlSource
{
public:
	MysqlSource(MysqlSource const&) = delete;
	void operator=(MysqlSource const&) = delete;
	~MysqlSource();

	static MysqlSource& getInstance();
	bool initialize(std::unique_ptr<CConfig>& config);

	// returns nullptr if failed to create a connection
	CMysqlHandler* getMysqlHandler();

private:
	bool initialized;

	std::string host;
	int port;
	std::string user;
	std::string password;
	std::string database;

	MysqlSource();
};
