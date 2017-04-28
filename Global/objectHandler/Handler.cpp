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
		mnMsqKey(createMsqKey()), mnMsqId(-1), pHandleMessage(0), mpInstance(0)
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

void Handler::onHandleMessage(Message &message)
{
	if(pHandleMessage && mpInstance)
		(*pHandleMessage)(message.what, message.arg1, message.arg2, message.obj, mpInstance);
}

void Handler::runMessageReceive()
{
	run(mnMsqId, "Handler");
	threadExit();
	threadJoin(getThreadID());
	_log("[Handler] runMessageReceive Stop, Thread join");
}

void Handler::setHandleMessageListener(void *pInstance, pfnHandleMessage handleMessage)
{
	mpInstance = pInstance;
	pHandleMessage = handleMessage;
}

int Handler::sendMessage(Message &message)
{
	return sendHandleMessage(mnMsqId, message);
}
