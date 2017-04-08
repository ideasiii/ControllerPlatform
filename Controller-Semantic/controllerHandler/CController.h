/*
 * CController.h
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CController: public CApplication
{
public:
	CController();
	virtual ~CController();

protected:
	void onInitial();
	void onFinish();

};
