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
#include "packet.h"
#include "JSONObject.h"

using namespace std;

CController::CController() :
		mnMsqKey(-1), cmpword(0), semanticJudge(0)
{
	semanticJudge = new CSemanticJudge(_OBJ(this));
	cmpword = new CCmpWord(_OBJ(this));
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
	string strConfPath = reinterpret_cast<const char*>(szConfPath);
	_DBG("[CController] onInitial Config File: %s", strConfPath.c_str());
	if(strConfPath.empty())
		return FALSE;

	int nPort;
	string strPort;
	CConfig *config;
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
	return TRUE;
}

int CController::onFinish(void* nMsqKey)
{
	cmpword->stop();
	delete cmpword;
	delete semanticJudge;
	return TRUE;
}

int CController::startCmpWordServer(int nPort, int nMsqKey)
{
	return cmpword->start(0, nPort, nMsqKey);
}

void CController::onHandleMessage(Message &message)
{
	_log("[CController] onHandleMessage what: %d obj: %s", message.what, reinterpret_cast<const char*>(message.obj));

	int nId = -1;
	string strWord;
	JSONObject* jsonResp;

	strWord = reinterpret_cast<const char*>(message.obj);

	if(strWord.empty())
	{
		cmpword->response(message.arg1, semantic_word_request, STATUS_RINVJSON, message.arg2, 0);
		return;
	}

	jsonResp = new JSONObject();

	switch(message.what)
	{
	case 0: // 語意判斷
		semanticJudge->word(strWord.c_str(), jsonResp);
		break;
	case 1: // 控制

		break;
	case 2: // 會話

		break;
	case 3: // 紀錄

		break;
	default:
		jsonResp->put("type", 0);
		break;
	}

	jsonResp->put("id", message.opt);
	cmpword->response(message.arg1, semantic_word_request, STATUS_ROK, message.arg2, jsonResp->toString().c_str());
}
