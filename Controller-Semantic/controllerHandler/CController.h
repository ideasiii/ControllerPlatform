/*
 * CController.h
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CCmpWord;
class Handler;

class CController: public CApplication
{
public:
	CController();
	virtual ~CController();
	int handleMessage(int what, int arg1, int arg2, void *obj);

protected:
	int onCreated(void* nMsqKey);
	/**
	 *  Main Process run will callback onInitial
	 */
	int onInitial(void* szConfPath);

	/*
	 *  Main Process terminator will callback onFinish
	 */
	int onFinish(void* nMsqKey);

private:
	int startCmpWordServer(int nPort, int nMsqKey);

private:
	CCmpWord *cmpword;
	int mnMsqKey;
	Handler *handler;
};
