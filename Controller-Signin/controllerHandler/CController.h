/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"
#include <string>

class CCmpSignin;
class CMysqlHandler;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int startSignin(const char *szIP, const int nPort, const int nMsqId);
	int stop();
	void setMysql(const char *szHost, const char *szPort, const char *szDB, const char *szUser, const char *szPassword);
	void runMysqlExec(std::string strSQL);
	void onSignin(const char *szData);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
	void onTimer(int nId);

private:
	explicit CController();
	CCmpSignin *cmpSignin;
	CMysqlHandler *mysql;

};
