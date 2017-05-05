/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CDispatcher;

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);

private:
	int mnMsqKey;
	int startDispatcher(const int nPort, int nMsqKey);
	CDispatcher *dispatcher;

};
