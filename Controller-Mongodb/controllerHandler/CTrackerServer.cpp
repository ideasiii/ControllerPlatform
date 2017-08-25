/*
 * CTrackerServer.cpp
 *
 *  Created on: 2017年3月31日
 *      Author: Jugo
 */

#include <string>
#include "CTrackerServer.h"
#include "common.h"
#include "packet.h"
#include "JSONObject.h"

using namespace std;

CTrackerServer::CTrackerServer(CObject *object)
{
	mpController = object;
}

CTrackerServer::~CTrackerServer()
{

}

int CTrackerServer::onAccesslog(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	const char *pBody = reinterpret_cast<const char*>(szBody);
	bool bValid;
	JSONObject *jsonObj = new JSONObject(pBody);
	bValid = jsonObj->isValid();
	jsonObj->release();
	delete jsonObj;

	if(!bValid)
	{
		return response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
	}
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);

	Message message;
	message.what = access_log_request;
	message.strData = pBody;
	mpController->sendMessage(message);
	return TRUE;
}
