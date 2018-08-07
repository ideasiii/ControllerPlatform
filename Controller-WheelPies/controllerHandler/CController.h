/*
 * CController.h
 *
 *  Created on: 2018年7月4日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CCmpWheelpies;
class CMysqlHandler;
class CMongoDBHandler;

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);

private:
	void insertData(std::string strData);

private:
	int mnMsqKey;
	CMysqlHandler *mysql;
	CCmpWheelpies *cmpwheelpies;
	CMongoDBHandler *mongodb;
};
