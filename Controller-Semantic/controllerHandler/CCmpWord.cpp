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

using namespace std;

#define MAX_SIZE		2048

CCmpWord::CCmpWord() :
		semanticJudge(0)
{
	semanticJudge = new CSemanticJudge();
}

CCmpWord::~CCmpWord()
{
	delete semanticJudge;
}

int CCmpWord::onSemanticWord(int nSocket, int nCommand, int nSequence, const void *szData)
{
	//response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
	char *pBody = (char*) ((char *) const_cast<void*>(szData)); // + sizeof(CMP_HEADER));

	_log("[CCmpWord] onSemanticWord Body: %s", pBody);

	if(isValidStr((const char*) pBody, MAX_SIZE))
	{
		char temp[MAX_SIZE];
		memset(temp, 0, sizeof(temp));
		strcpy(temp, pBody);

		if(0 < strlen(temp))
		{
			WORD_REQUEST wordRequest;
			JSONObject *jobjRoot = new JSONObject(temp);
			if(jobjRoot->isValid())
			{
				wordRequest.nId = jobjRoot->getInt("id");
				wordRequest.nType = jobjRoot->getInt("type");
				wordRequest.nTotal = jobjRoot->getInt("total");
				wordRequest.nNumber = jobjRoot->getInt("number");
				wordRequest.strWord = jobjRoot->getString("word");
				_log("[CCmpWord] onSemanticWord: id: %d type: %d total: %d number: %d word: %s Socket[%d]",
						wordRequest.nId, wordRequest.nType, wordRequest.nTotal, wordRequest.nNumber,
						wordRequest.strWord.c_str(), nSocket);
			}
			jobjRoot->release();
			delete jobjRoot;

			if(0 > wordRequest.nId)
			{
				response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
				return FALSE;
			}

			JSONObject jsonResp;
			jsonResp.put("id", wordRequest.nId);
			switch(wordRequest.nType)
			{
			case 0: // 語意判斷
				semanticJudge->word(wordRequest.strWord.c_str(), &jsonResp);
				break;
			case 1: // 控制
				break;
			case 2: // 會話
				break;
			case 3: // 紀錄
				break;
			default:
				response(nSocket, nCommand, STATUS_RINVJSON, nSequence, 0);
				return FALSE;
			}
			response(nSocket, nCommand, STATUS_ROK, nSequence, jsonResp.toString().c_str());
			jsonResp.release();
		}
	}
	return TRUE;
}

