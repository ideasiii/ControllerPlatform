/*
 * CManager.h
 *
 *  Created on: 2017年8月18日
 *      Author: root
 */

#pragma once

#include "CCmpServer.h"

class CManager: public CCmpServer
{
public:
	explicit CManager(CObject *object);
	virtual ~CManager();

private:
	CObject *mpController;
};
