#pragma once

#include <memory>
#include <string>

class CMysqlHandler;
class CConfig;

// 關於 MySQL 資料庫的連線參數
class MysqlSource
{
public:
	MysqlSource(MysqlSource const&) = delete;
	void operator=(MysqlSource const&) = delete;
	~MysqlSource();

	static MysqlSource& getInstance();
	bool initialize(std::unique_ptr<CConfig>& config);

	// Get CMysqlHandler which has been already connected to DB
	// returns nullptr if failed to create a connection
	std::unique_ptr<CMysqlHandler> getMysqlHandler();

private:
	bool initialized;

	std::string host;
	int port;
	std::string user;
	std::string password;
	std::string database;
	std::string connTimeout;

	MysqlSource();
};
