/*
 * CController.h
 *
 *  Created on: 2017年8月18日
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

private:
	int mnMsqKey;
};
