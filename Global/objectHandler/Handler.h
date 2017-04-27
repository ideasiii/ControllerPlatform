/*
 * Handler.h
 *
 *  Created on: 2017年4月27日
 *      Author: Jugo
 */

#pragma once
#include "CObject.h"

typedef int (*pfnHandleMessage)(int, int, int, const void *, const void *);

class Handler: public CObject
{
public:
	explicit Handler(const int nMsqKey = -1);
	virtual ~Handler();
	void close();
	void runMessageReceive();
	void setHandleMessageListener(pfnHandleMessage handleMessage);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	int mnMsqKey;
	int mnMsqId;
	pfnHandleMessage pHandleMessage;
};
