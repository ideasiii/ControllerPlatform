/*
 * CCmpWheelpies.cpp
 *
 *  Created on: 2018年7月4日
 *      Author: root
 */

#include <string>
#include "CCmpWheelpies.h"
#include "packet.h"

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
	string strBody = string(reinterpret_cast<const char*>(szBody));
	if(!strBody.empty() && 0 < strBody.length())
	{
		_log("[CCmpWheelpies] onWheelpies Body: %s", szBody);
		response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
		return TRUE;
	}
	response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
	return FALSE;
}

