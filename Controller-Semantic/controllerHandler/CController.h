/*
 * CController.h
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CCmpWord;

class CController: public CApplication
{
public:
	CController();
	virtual ~CController();

protected:
	/**
	 *  Main Process run will callback onInitial
	 */
	void onInitial();

	/*
	 *  Main Process terminator will callback onFinish
	 */
	void onFinish();

private:
	int startCmpWordServer();

private:
	CCmpWord *cmpword;
};
