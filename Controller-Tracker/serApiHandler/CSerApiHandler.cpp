/*
 * CSerApiHandler.cpp
 *
 *  Created on: 2015年12月22日
 *      Author: Louis Ju
 */

#include "CSerApiHandler.h"
#include "../httpHandler/CHttpClient.h"
#include "SerAPI.h"
#include <map>
#include "../jsonHandler/cJSON.h"
#include "../global_inc/common.h"
#include <stdio.h>
#include <stdexcept>
using namespace std;

static CSerApiHandler *mInstance = 0;

CSerApiHandler::CSerApiHandler() :
		httpClient(new CHttpClient())
{

}

CSerApiHandler::~CSerApiHandler()
{
	delete httpClient;
}

CSerApiHandler *CSerApiHandler::getInstance()
{
	if (!mInstance)
	{
		mInstance = new CSerApiHandler();
	}

	return mInstance;
}

int CSerApiHandler::signin(string strAccount, string strPassword)
{
	int nRet = HTTP_OK;
	string strParam;
	map<string, string> mapData;

	strParam = "account=" + strAccount + "&password=" + strPassword;
	nRet = httpClient->post( SER_API_SIGNIN_HOST, 80, SER_API_SIGNIN_PAGE, strParam, mapData);

	if (HTTP_OK == nRet)
		return SIGN_IN_OK;
	return SIGN_IN_FAIL;
}
int CSerApiHandler::insertEvent(string token, string jsonInputString)
{
	if(token.empty())
		return INSERT_EVENT_FAIL;

	string inputParm = "token=";
	inputParm = inputParm + token +"&";
	//printf("json input data %s \n", inputParm.c_str());

	//use simple json type
	cJSON *cJsonInputString = cJSON_Parse(jsonInputString.c_str());
	cJSON *data = cJsonInputString->child;
	cJSON *next;

	while (data)
	{
		next = data->next;
		inputParm = inputParm + data->string;
		inputParm = inputParm + "=";
		inputParm = inputParm + data->valuestring;

		data = next;
		if (data)
		{
			inputParm += "&";
		}
	}
	printf("json input data %s \n", inputParm.c_str());

	string jsonData = getHttpJsonData(SER_API_HOST, SER_API_INSERT_EVENT_PAGE,  inputParm);
	if(!jsonData.empty())
	{
		cJSON *returnData = cJSON_Parse(jsonData.c_str());
		if (cJSON_GetObjectItem(returnData, "message")
				&& (cJSON_String == cJSON_GetObjectItem(returnData, "message")->type))
		{
			string message = cJSON_GetObjectItem(returnData, "message")->valuestring;
			if(message.compare("success") == 0)
			{
				_DBG("[SER API] insert success!")
				return INSERT_EVENT_OK;
			}
			else
			{
				_DBG("[SER API] insert fail: %s",message.c_str())
				return INSERT_EVENT_FAIL;
			}
		}
	}
	_DBG("[SER API] insert fail: return data is empty")
	return INSERT_EVENT_FAIL;
}

string CSerApiHandler::getHttpJsonData(string host, string apiFunc, string inputParm)
{
	int nRet = HTTP_OK;
	string jsonData = "";
	map<string, string> *mapData = new map<string, string>();
	nRet = this->httpClient->post(host, 80, apiFunc, inputParm, *mapData);
	printf("nRet is %d\n", nRet);
	if (nRet == HTTP_OK)
	{
		try
		{
			string body = mapData->find("body")->second ;

			//find most left { and most right }
			int pos = body.find('{');
			string leaf = body.substr(pos);
			jsonData = leaf.substr(0, leaf.find_last_of('}') + 1);
		}
		catch ( std::out_of_range& exception)
		{
			_DBG("[SER API] string out_of_range")
		}
	}

	mapData->clear();
	delete mapData;

	return jsonData;
}


int CSerApiHandler::updateSERToken()
{
	string tmpToken = getToken();
	if(!tmpToken.empty())
	{
		serApiToken = tmpToken;
		return UPDATE_SER_TOKEN_OK;
	}
	else
	{
		return UPDATE_SER_TOKEN_FAIL;
	}
}
string CSerApiHandler::getToken()
{

	string id = SER_API_DEVELOPER_ID;
	string passwd = SER_API_DEVELOPER_PASSWORD;
	string strParam = "id=" + id + "&secret_key=" + passwd;

	string jsonData = getHttpJsonData(SER_API_HOST, SER_API_GET_TOKEN_PAGE, strParam);

	if (!jsonData.empty())
	{
		printf("json data: %s",jsonData.c_str());
		cJSON *returnData = cJSON_Parse(jsonData.c_str());
		if (returnData)
		{
			//success get token
			/*{
			 "message": "success",
			 "result":
			 {
			 "token": "取得的token",
			 "token_expire": "到期時間",
			 "access_time": "取得時間"
			 }
			 }*/
			if (cJSON_GetObjectItem(returnData, "result")
					&& (cJSON_Object == cJSON_GetObjectItem(returnData, "result")->type))
			{
				cJSON *tokenData = cJSON_GetObjectItem(returnData, "result");
				if (cJSON_GetObjectItem(tokenData, "token")
						&& (cJSON_String == cJSON_GetObjectItem(tokenData, "token")->type))
				{
					//printf("token: %s\n", cJSON_GetObjectItem(tokenData, "token")->valuestring);
					return cJSON_GetObjectItem(tokenData, "token")->valuestring;
				}
			}

		}
		else
		{
			_DBG("[SER API] error to get token")
		}
	}

	return "";
}

