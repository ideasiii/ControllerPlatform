/*
 * CCmpWheelpies.cpp
 *
 *  Created on: 2018年7月4日
 *      Author: root
 */

#include <string>
#include "CCmpWheelpies.h"
#include "packet.h"
#include "JSONObject.h"

using namespace std;

CCmpWheelpies::CCmpWheelpies(CObject *object)
{
	mpController = object;
}

CCmpWheelpies::~CCmpWheelpies()
{
	// TODO Auto-generated destructor stub
}

int CCmpWheelpies::onWheelpies(int nSocket, int nCommand, int nSequence, const void *szBody)
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
	message.what = wheelpies_request;
	message.strData = pBody;
	mpController->sendMessage(message);

	return FALSE;
}

