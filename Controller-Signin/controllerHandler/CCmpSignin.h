/*
 * CCmpSignin.h
 *
 *  Created on: 2017年3月16日
 *      Author: root
 */

#pragma once

#include "CCmpServer.h"
#include "ICallback.h"
#include <map>

class CObject;

class CCmpSignin: public CCmpServer
{
public:
	explicit CCmpSignin(CObject *object);
	virtual ~CCmpSignin();

protected:
	int onSignin(int nSocket, int nCommand, int nSequence, const void *szData);

private:
	CObject * mpController;
};
