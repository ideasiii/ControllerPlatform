/*
 * CCmpWord.cpp
 *
 *  Created on: 2017年4月10日
 *      Author: Jugo
 */

#include <string>
#include "CCmpWord.h"
#include "packet.h"
#include "utility.h"
#include "common.h"
#include "JSONObject.h"
#include "CSemanticJudge.h"
#include "config.h"

using namespace std;

CCmpWord::CCmpWord(CObject *object) :
		mpController(0)
{
	mpController = object;
}

CCmpWord::~CCmpWord()
{

}

int CCmpWord::onSemanticWord(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	string strBody = string(reinterpret_cast<const char*>(szBody));
	if(!strBody.empty() && 0 < strBody.length())
	{
		_log("[CCmpWord] onSemanticWord Body: %s", szBody);
		WORD_REQUEST wordRequest;
		JSONObject *jobjRoot = new JSONObject(strBody);
		if(jobjRoot->isValid())
		{
			wordRequest.nId = jobjRoot->getInt("id");
			wordRequest.nType = jobjRoot->getInt("type");
			wordRequest.nTotal = jobjRoot->getInt("total");
			wordRequest.nNumber = jobjRoot->getInt("number");
			wordRequest.strWord = jobjRoot->getString("word");
			wordRequest.strDeviceId = jobjRoot->getString("device_id");
		}
		jobjRoot->release();
		delete jobjRoot;

		if(0 > wordRequest.nId || 0 > wordRequest.nType || TYPE_REQ_MAX <= wordRequest.nType
				|| wordRequest.strWord.empty())
		{
			response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
			return FALSE;
		}

		Message message;
		message.clear();
		message.what = semantic_word_request;
		message.arg[0] = nSocket;
		message.arg[1] = nSequence;
		message.arg[2] = wordRequest.nId;
		message.arg[3] = wordRequest.nType;
		message.strData = strBody; //wordRequest.strWord;
		mpController->sendMessage(message);
	}
	else
		response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
	return TRUE;
}

int CCmpWord::onUpdate(int nSocket, int nCommand, int nSequence, const void *szBody)
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

