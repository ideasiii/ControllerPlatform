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

class CCmpSignin: public CCmpServer
{
public:
	CCmpSignin();
	virtual ~CCmpSignin();
	void setCallback(const int nId, CBFun cbfun);

protected:
	int onSignin(int nSocket, int nCommand, int nSequence, const void *szData);

private:
	std::map<int, CBFun> mapCallback;
};
