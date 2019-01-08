/*
 * Controller.h
 *
 *  Created on: 2019年01月07日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

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
	void onTimer(int nId);

private:
	void foldScan();

private:
	int mnMsqKey;

};
