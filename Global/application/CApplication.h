/*
 * CApplication.h
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"
#include <map>

class CApplication: public CObject
{
public:
	explicit CApplication();
	virtual ~CApplication();
	void callback(int nMsg);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

protected:
	virtual void onInitial() = 0;
	virtual void onFinish() = 0;

private:
	typedef void (CApplication::*MemFn)(void);
	std::map<int, MemFn> mapFunc;
};
