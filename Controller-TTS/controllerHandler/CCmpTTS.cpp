/*
 * CCmpWord.cpp
 *
 *  Created on: 2017年4月10日
 *      Author: Jugo
 */

#include "../controllerHandler/CCmpTTS.h"

#include <string>
#include "packet.h"
#include "utility.h"
#include "common.h"
#include "JSONObject.h"


using namespace std;

CCmpTTS::CCmpTTS(CObject *object) :
		mpController(0)
{
	mpController = object;
}

CCmpTTS::~CCmpTTS()
{

}

int CCmpTTS::onTTS(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	string strBody = string(reinterpret_cast<const char*>(szBody));
	if(!strBody.empty() && 0 < strBody.length())
	{
		_log("[CCmpTTS] onTTS Body: %s", szBody);

		Message message;
		message.clear();
		message.what = semantic_word_request;
		message.strData = strBody; //wordRequest.strWord;
		mpController->sendMessage(message);
	}
	else
		response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
	return TRUE;
}

int CCmpTTS::onUpdate(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
	Message message;
	message.clear();
	message.what = semantic_word_request;
	message.arg[0] = nSocket;
	message.arg[1] = nSequence;
	mpController->sendMessage(message);
	return TRUE;
}

