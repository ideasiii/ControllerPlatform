/*
 * CController.cpp
 *
 *  Created on: 2017年03月14日
 *      Author: Jugo
 */

#include <map>
#include "common.h"
#include "LogHandler.h"
#include "CController.h"
#include "CCmpSignin.h"
#include "event.h"
#include "callback.h"
#include "CMysqlHandler.h"
#include "JSONObject.h"

using namespace std;

#define COMMAND_ON_SIGNIN		9000

static CController * controller = 0;

/** Callback Function send SQL to message queue then run it **/
void _onSignin(void* param)
{
	string strParam = reinterpret_cast<const char*>(param);
	controller->sendMessage(EVENT_FILTER_CONTROLLER, COMMAND_ON_SIGNIN, 0, strParam.length(), strParam.c_str());
}

CController::CController() :
		CObject(), cmpSignin(new CCmpSignin()), mysql(new CMysqlHandler)
{

}

CController::~CController()
{
	delete mysql;
	delete cmpSignin;
}

CController* CController::getInstance()
{
	if(0 == controller)
	{
		controller = new CController();
	}
	return controller;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch(nCommand)
	{
	case COMMAND_ON_SIGNIN:
		onSignin((char *) const_cast<void*>(pData));
		break;
	}
}

void CController::onTimer(int nId)
{

}

int CController::startSignin(const char *szIP, const int nPort, const int nMsqId)
{
	if(cmpSignin->start(szIP, nPort))
	{
		cmpSignin->setCallback(CB_RUN_MYSQL_SQL, _onSignin);
		cmpSignin->idleTimeout(true, 30);
		return TRUE;
	}
	return FALSE;
}

int CController::stop()
{
	cmpSignin->stop();
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
