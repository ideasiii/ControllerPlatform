/*
 * CController.h
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 *      this is dummy class
 */

#pragma once

#include "CApplication.h"

class CController: public CApplication
{
public:
	CController();
	virtual ~CController();

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);

};
