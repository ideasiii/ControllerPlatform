#pragma once

#include <memory>

class CClientAmxController
{
public:
	explicit CClientAmxController(const std::string &serverIp, int userPort, int validationPort);
	virtual ~CClientAmxController();

	std::string getServerIp();
	int getUserPort();
	int getValidationPort();

private:
	const std::string serverIp;
	const int userPort;
	const int validationPort;
};
