/*
 * CController.cpp
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */
#include <string>
#include "CController.h"
#include "CCmpWord.h"
#include "common.h"
#include "CConfig.h"
#include "utility.h"
#include "event.h"
#include "Handler.h"
#include "CObject.h"
#include "CSemanticJudge.h"
#include "CSemanticControl.h"
#include "CSemanticTalk.h"
#include "CSemanticRecord.h"
#include "packet.h"
#include "JSONObject.h"
#include "CFileHandler.h"
#include "dic_music_artist_female_en.h"

using namespace std;

CController::CController() :
		mnMsqKey(-1)
{
	semanticJudge = new CSemanticJudge(_OBJ(this));
	semanticControl = new CSemanticControl(_OBJ(this));
	semanticTalk = new CSemanticTalk(_OBJ(this));
	semanticRecord = new CSemanticRecord(_OBJ(this));
	cmpword = new CCmpWord(this);
}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_SEMANTIC; //*(reinterpret_cast<int*>(nMsqKey));
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nCount;
	int nPort;
	string strConfPath;
	string strPort;
	CConfig *config;
	CFileHandler fh;

	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if(strConfPath.empty())
		return FALSE;

	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		strPort = config->getValue("SERVER WORD", "port");
		if(!strPort.empty())
		{
			convertFromString(nPort, strPort);
			startCmpWordServer(nPort, mnMsqKey);
		}
	}
	delete config;

	//============== Load Dictionary =================//
	nCount = fh.readAllLine("dictionary/artist_female_en.txt", setArtistEnglishFemale);
	_log("[CController] onInitial Load artist female en: %u", setArtistEnglishFemale.size());
	if(!nCount)
		return FALSE;

	return TRUE;
}

int CController::onFinish(void* nMsqKey)
{
	cmpword->stop();
	delete cmpword;
	delete semanticJudge;
	delete semanticControl;
	delete semanticTalk;
	delete semanticRecord;
	return TRUE;
}

int CController::startCmpWordServer(int nPort, int nMsqKey)
{
	return cmpword->start(0, nPort, nMsqKey);
}

void CController::onHandleMessage(Message &message)
{
	int nWhat;
	int nId;
	int nSocket;
	int nSequence;
	string strWord;
	JSONObject* jsonResp;

	nSocket = message.arg[0];
	nSequence = message.arg[1];
	nId = message.arg[2];
	nWhat = message.what;
	strWord = message.strData;

	if(strWord.empty())
	{
		cmpword->response(nSocket, semantic_word_request, STATUS_RINVJSON, nSequence, 0);
		return;
	}

	jsonResp = new JSONObject();

	switch(nWhat)
	{
	case 0: // 語意判斷
		semanticJudge->word(strWord.c_str(), jsonResp);
		break;
	case 1: // 控制
		semanticControl->word(strWord.c_str(), jsonResp);
		break;
	case 2: // 會話
		semanticTalk->word(strWord.c_str(), jsonResp);
		break;
	case 3: // 紀錄
		semanticRecord->word(strWord.c_str(), jsonResp);
		break;
	default:
		jsonResp->put("type", 0);
		break;
	}

	jsonResp->put("id", nId);
	cmpword->response(nSocket, semantic_word_request, STATUS_ROK, nSequence, jsonResp->toString().c_str());
}
