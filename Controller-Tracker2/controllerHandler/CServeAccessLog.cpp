#include "IReceiver.h"
#include "event.h"
#include "common.h"

#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "CServerAccessLog.h"
#include "iCommand.h"
#include "packet.h"

CServerAccessLog::CServerAccessLog(CObject *object)
{
	mpController = object;
	runIdleTimeout(true);
}

CServerAccessLog::~CServerAccessLog()
{
}

int CServerAccessLog::onAccesslog(int nSocket, int nCommand, int nSequence, const void *szBody)
{

	_DBG("[CServerAccessLog]cmpAccessLogRequest");
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);

	string data = paseBody(szBody);

	//_log("CServerAccessLog]#####onAccesslog##### %s", data.c_str());

	if (!data.empty())
	{
		//mongoDB

		Message message;
		message.what = MESSAGE_EVENT_DEVICE_SERVER;
		message.strData = data.c_str();
		mpController->sendMessage(message);


	}
	else
	{
		_log("[CServerAccessLog]Error while covert access log!");

	}

	return TRUE;

}


string CServerAccessLog::paseBody(const void *szBody)
{
	const char *pBody = reinterpret_cast<const char*>(szBody);
	int nType = ntohl(*((int*) pBody));
	pBody += 4;

	return pBody;

}

