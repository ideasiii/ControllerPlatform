/*
 * CServerAMX.h
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#pragma once

#include "CAMXServer.h"

class CServerAMX: public CAMXServer
{
public:
	explicit CServerAMX(CObject *object);
	virtual ~CServerAMX();

protected:
	int onAmxStatus(unsigned long int nSocketFD, const char *szStatus);

private:
	CObject *mpController;
};
