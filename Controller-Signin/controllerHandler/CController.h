/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CCmpSignin;
class CMysqlHandler;

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();

	void runMysqlExec(std::string strSQL);
	void onSignin(const char *szData);

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);

private:
	void setMysql(const char *szHost, const char *szPort, const char *szDB, const char *szUser, const char *szPassword);
	int startSignin(const int nPort, const int nMsqId);
	CCmpSignin *cmpSignin;
	CMysqlHandler *mysql;
	int mnMsqKey;

};
