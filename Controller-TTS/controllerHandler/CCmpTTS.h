/*
 * CCmpWord.h
 *
 *  Created on: 2018年10月01日
 *      Author: Jugo
 */

#pragma once

#include "CCmpServer.h"

class CCmpTTS: public CCmpServer
{

public:
	explicit CCmpTTS(CObject *object);
	virtual ~CCmpTTS();

protected:
	int onTTS(int nSocket, int nCommand, int nSequence, const void *szBody);

private:
	CObject *mpController;
};
