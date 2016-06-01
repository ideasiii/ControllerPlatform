/*
 * CSerApiHandler.h
 *
 */

#pragma once
#include <string>
#include <map>
enum
{
	SIGN_IN_OK, SIGN_IN_FAIL, INSERT_EVENT_OK, INSERT_EVENT_FAIL, UPDATE_SER_TOKEN_OK, UPDATE_SER_TOKEN_FAIL
};

class CHttpClient;
class cJSON;

class CSerApiHandler
{
public:
	static CSerApiHandler *getInstance();
	virtual ~CSerApiHandler();
	int signin(std::string strAccount, std::string strPassword);
	std::string getToken();
	int insertEvent(std::string token, std::string jsonData);
	int updateSERToken();
	std::string serApiToken;

private:
	std::string getHttpJsonData(std::string, std::string, std::string);
	explicit CSerApiHandler();
	CHttpClient *httpClient;

};
