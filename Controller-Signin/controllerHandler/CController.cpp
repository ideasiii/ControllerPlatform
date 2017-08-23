/*
 * CController.cpp
 *
 *  Created on: 2017年03月14日
 *      Author: Jugo
 */

#include <string>
#include <map>
#include "common.h"
#include "CController.h"
#include "CCmpSignin.h"
#include "event.h"
#include "CMysqlHandler.h"
#include "JSONObject.h"
#include "CConfig.h"
#include "utility.h"
#include "packet.h"

using namespace std;

CController::CController() :
		cmpSignin(0), mysql(0), mnMsqKey(-1)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	cmpSignin = new CCmpSignin(this);
	mysql = new CMysqlHandler;
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_SIGNIN;
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nPort;
	string strPort;
	CConfig *config;
	string strConfPath;

	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if(strConfPath.empty())
		return FALSE;

	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		strPort = config->getValue("SERVER SIGNIN", "port");
		if(!strPort.empty())
		{
			convertFromString(nPort, strPort);
			startSignin(nPort, mnMsqKey);
		}

		setMysql(config->getValue("MYSQL", "host").c_str(), config->getValue("MYSQL", "port").c_str(),
				config->getValue("MYSQL", "database").c_str(), config->getValue("MYSQL", "user").c_str(),
				config->getValue("MYSQL", "password").c_str());
	}
	delete config;
	return TRUE;
}

int CController::onFinish(void* nMsqKey)
{
	cmpSignin->stop();
	delete cmpSignin;
	delete mysql;
	return TRUE;
}

int CController::startSignin(const int nPort, const int nMsqId)
{
	if(cmpSignin->start(0, nPort, nMsqId))
	{
		cmpSignin->idleTimeout(true, 3);
		return TRUE;
	}
	return FALSE;
}

void CController::setMysql(const char *szHost, const char *szPort, const char *szDB, const char *szUser,
		const char *szPassword)
{
	extern map<string, string> mapMysqlSetting;
	mapMysqlSetting["host"] = szHost;
	mapMysqlSetting["port"] = szPort;
	mapMysqlSetting["database"] = szDB;
	mapMysqlSetting["user"] = szUser;
	mapMysqlSetting["password"] = szPassword;
}

void CController::runMysqlExec(std::string strSQL)
{
	_log("[CController] Run Mysql SQL: %s", strSQL.c_str());
	extern map<string, string> mapMysqlSetting;

	int nRet = mysql->connect(mapMysqlSetting["host"], mapMysqlSetting["database"], mapMysqlSetting["user"],
			mapMysqlSetting["password"]);

	if(FALSE == nRet)
	{
		_log("[CController] Mysql Error: %s", mysql->getLastError().c_str());
		return;
	}

	if(FALSE == mysql->sqlExec(strSQL))
	{
		if(1062 != mysql->getLastErrorNo())
			_log("[CController] Mysql sqlExec Error: %s", mysql->getLastError().c_str());
	}

	mysql->close();
}

void CController::onSignin(const char *szData)
{
	if(0 == szData)
		return;

	JSONObject jsonData(szData);
	if(jsonData.isValid())
	{
		if(!jsonData.getString("id").empty())
		{
			string strSQL =
					"INSERT INTO tracker_user(id,app_id,mac,os,phone,fb_id,fb_name,fb_email,fb_account,g_account,t_account) VALUES('"
							+ jsonData.getString("id") + "','" + jsonData.getString("app_id") + "','"
							+ jsonData.getString("mac") + "','" + jsonData.getString("os") + "','"
							+ jsonData.getString("phone") + "','" + jsonData.getString("fb_id") + "','"
							+ jsonData.getString("fb_name") + "','" + jsonData.getString("fb_email") + "','"
							+ jsonData.getString("fb_account") + "','" + jsonData.getString("g_account") + "','"
							+ jsonData.getString("t_account") + "');";
			runMysqlExec(strSQL);
		}
	}
	jsonData.release();
}

void CController::onHandleMessage(Message &message)
{
	switch(message.what)
	{
	case sign_up_request:
		onSignin(message.strData.c_str());
		break;
	case controller_die_request:
		JSONObject jsonData(message.strData);
		_log("[CController] onHandleMessage get die request message: %s", message.strData.c_str());
		if(!jsonData.getString("key").compare("suicide"))
		{
			_log("[CController] onHandleMessage ...主人 我要去死了.... 感謝您的關照 謝謝!!!");
			terminateController();
		}
		break;
	}
}
