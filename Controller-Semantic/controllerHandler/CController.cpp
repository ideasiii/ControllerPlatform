/*
 * CController.cpp
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */
#include <string>
#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
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
#include "config.h"

using namespace std;

CController::CController() :
		mnMsqKey(-1), cmpword(0), semanticJudge(0), semanticControl(0), semanticTalk(0), semanticRecord(0)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_SEMANTIC;
	semanticJudge = new CSemanticJudge(this);
	semanticControl = new CSemanticControl(this);
	semanticTalk = new CSemanticTalk(this);
	semanticRecord = new CSemanticRecord(this);
	cmpword = new CCmpWord(this);
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nResult;
	int nCount;
	int nPort;
	string strConfPath;
	string strPort;
	CConfig *config;

	nResult = FALSE;
	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if(strConfPath.empty())
		return nResult;

	config = new CConfig();
	if(config->loadConfig(strConfPath))
	{
		strPort = config->getValue("SERVER WORD", "port");
		if(!strPort.empty())
		{
			convertFromString(nPort, strPort);
			nResult = cmpword->start(0, nPort, mnMsqKey);
		}
	}
	delete config;

	return nResult;
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

void CController::onSemanticWordRequest(const int nSocketFD, const int nSequence, const int nId, const int nType,
		const char *szWord)
{
	JSONObject jsonResp;
	jsonResp.create();
	string strResp;

	switch(nType)
	{
	case TYPE_REQ_NODEFINE: // 語意判斷
		semanticJudge->word(szWord, jsonResp);
		break;
	case TYPE_REQ_CONTROL:	// 控制
		semanticControl->word(szWord, jsonResp);
		break;
	case TYPE_REQ_TALK: 	// 會話
		semanticTalk->word(szWord, jsonResp);
		break;
	case TYPE_REQ_RECORD:	// 紀錄
		semanticRecord->word(szWord, jsonResp);
		break;
	default:
		cmpword->response(nSocketFD, semantic_word_request, STATUS_RINVJSON, nSequence, 0);
		return;
	}
	jsonResp.put("id", nId);
	strResp = jsonResp.toJSON();
	jsonResp.release();
	cmpword->response(nSocketFD, semantic_word_request, STATUS_ROK, nSequence, strResp.c_str());
}

void CController::onHandleMessage(Message &message)
{
	switch(message.what)
	{
	case semantic_word_request:
		// Lambda Expression
		thread([=]
		{	onSemanticWordRequest( message.arg[0], message.arg[1], message.arg[2], message.arg[3],
					message.strData.c_str());}).detach();

//		auto lambda = [=]
//		{	onSemanticWordRequest( message.arg[0], message.arg[1], message.arg[2], message.arg[3],
//					message.strData.c_str());};
//		thread(lambda).detach();

//		onSemanticWordRequest(message.arg[0], message.arg[1], message.arg[2], message.arg[3], message.strData.c_str());
		break;
	}
}
