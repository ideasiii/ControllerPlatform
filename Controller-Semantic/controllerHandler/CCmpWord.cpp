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

using namespace std;

#define MAX_SIZE		2048

CCmpWord::CCmpWord()
{
}

CCmpWord::~CCmpWord()
{
}

int CCmpWord::onSemanticWord(int nSocket, int nCommand, int nSequence, const void *szData)
{
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
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
		}
	}
	return TRUE;
}

