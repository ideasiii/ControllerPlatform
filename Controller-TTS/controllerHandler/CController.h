/*
 * CController.h
 *
 *  Created on: 2018年9月27日
 *      Author: Jugo
 *
 */

#pragma once

#include "CApplication.h"

class CTextProcess;

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

private:
	int mnMsqKey;
	CTextProcess *textProcess;
};
