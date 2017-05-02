/*
 * CController.h
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CSemanticJudge;
class CCmpWord;

class CController: public CApplication
{
public:
	CController();
	virtual ~CController();

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
	void onHandleMessage(Message &message);

private:
	int startCmpWordServer(int nPort, int nMsqKey);

private:
	int mnMsqKey;
	CCmpWord *cmpword;
	CSemanticJudge *semanticJudge;
};
