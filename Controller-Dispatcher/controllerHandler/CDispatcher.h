/*
 * CDispatcher.h
 *
 *  Created on: 2017年3月7日
 *      Author: Jugo
 */

#pragma once

#include "CCmpServer.h"

class CDispatcher: public CCmpServer
{
public:
	static CDispatcher* getInstance();
	virtual ~CDispatcher();

protected:
	int onInitial(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onDie(int nSocket, int nCommand, int nSequence, const void *szBody);

private:
	explicit CDispatcher();

};
