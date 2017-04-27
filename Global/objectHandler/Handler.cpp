/*
 * Handler.cpp
 *
 *  Created on: 2017年4月27日
 *      Author: root
 */

#include <sys/time.h>
#include "Handler.h"
#include "event.h"
#include "CMessageHandler.h"
#include "common.h"

void *threadHandlerMessageReceive(void *argv)
{
	Handler* ss = reinterpret_cast<Handler*>(argv);
	ss->runMessageReceive();
	return 0;
}

inline double createMsqKey()
{
	timeval tv;
	gettimeofday(&tv, NULL);
	return double(tv.tv_sec) + 0.000001 * tv.tv_usec;
}

Handler::Handler(const int nMsqKey) :
		mnMsqKey(createMsqKey()), mnMsqId(-1)
{
	close();
	if(-1 != nMsqKey)
		mnMsqKey = nMsqKey;
	mnMsqId = initMessage(mnMsqKey, "Handler");
	if(0 < mnMsqId)
		createThread(threadHandlerMessageReceive, this, "Handler Message Receive Thread");
}

Handler::~Handler()
{
	close();
}

void Handler::close()
{
	if(-1 != mnMsqId)
		CMessageHandler::closeMsg(mnMsqId);
}

void Handler::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	(*pHandleMessage)(nCommand, nId, nDataLen, pData, pData);
}

void Handler::runMessageReceive()
{
	run(mnMsqId, "Handler");
	threadExit();
	threadJoin(getThreadID());
	_log("[Handler] runMessageReceive Stop, Thread join");
}

void Handler::setHandleMessageListener(pfnHandleMessage handleMessage)
{
	pHandleMessage = handleMessage;
}

