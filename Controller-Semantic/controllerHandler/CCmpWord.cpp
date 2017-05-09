/*
 * CCmpWord.cpp
 *
 *  Created on: 2017年4月10日
 *      Author: root
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

//	const char *pBody = reinterpret_cast<const char*>(szBody);
//	Message message;
//	message.what = TYPE_REQ_NODEFINE;
//	message.strData = pBody;
//	mpController->sendMessage(message);
//	return 1;

	string strBody = string(reinterpret_cast<const char*>(szBody));
	if(!strBody.empty() && 0 < strBody.length())
	{
		WORD_REQUEST wordRequest;
		JSONObject *jobjRoot = new JSONObject(strBody);
		if(jobjRoot->isValid())
		{
			wordRequest.nId = jobjRoot->getInt("id");
			wordRequest.nType = jobjRoot->getInt("type");
			wordRequest.nTotal = jobjRoot->getInt("total");
			wordRequest.nNumber = jobjRoot->getInt("number");
			wordRequest.strWord = jobjRoot->getString("word");
			_log("[CCmpWord] onSemanticWord: id: %d type: %d total: %d number: %d word: %s Socket[%d]", wordRequest.nId,
					wordRequest.nType, wordRequest.nTotal, wordRequest.nNumber, wordRequest.strWord.c_str(), nSocket);
		}
		jobjRoot->release();
		delete jobjRoot;

		if(0 > wordRequest.nId)
		{
			response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
			return FALSE;
		}

		Message message;
		switch(wordRequest.nType)
		{
		case 0: // 語意判斷
			message.what = TYPE_REQ_NODEFINE;
			break;
		case 1: // 控制
			message.what = TYPE_REQ_CONTROL;
			break;
		case 2: // 會話
			message.what = TYPE_REQ_TALK;
			break;
		case 3: // 紀錄
			message.what = TYPE_REQ_RECORD;
			break;
		default:
			response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
			return FALSE;
		}
		message.arg1 = nSocket;
		message.arg2 = nSequence;
		message.strData = wordRequest.strWord;
		mpController->sendMessage(message);
	}
	return TRUE;
}

