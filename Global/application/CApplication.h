/*
 * CApplication.h
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"
#include <map>
#include <string>

class CApplication: public CObject
{
public:
	explicit CApplication();
	virtual ~CApplication();
	int callback(int nMsg, void* param);
	void setConfPath(const char * szPath);
	std::string getConfPath();
	void terminateController();

protected:
	virtual void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

protected:
	virtual int onCreated(void* nMsqKey) = 0;
	virtual int onInitial(void* szConfPath) = 0;
	virtual int onFinish(void* nMsqKey) = 0;
	virtual void onHandleMessage(Message &message);

private:
	typedef int (CApplication::*MemFn)(void*);
	std::map<int, MemFn> mapFunc;
	std::string mstrConfPath;
};
