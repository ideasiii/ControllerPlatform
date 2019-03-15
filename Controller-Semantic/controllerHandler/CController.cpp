/*
 * CController.cpp
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */
#include <string>
#include<fstream>
#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
#include "config.h"
#include "CPenReader.h"
#include "CMysqlHandler.h"
#include "CString.h"
#include "CStory.h"
#include "CParticiple.h"
#include "CChihlee.h"

#define STORY_FILE_PATH			"/data/opt/tomcat/webapps/story/"

using namespace std;

CController::CController() :
		mnMsqKey(-1), cmpword(0), semanticJudge(0), penreader(0), mysql(0), chihlee(0)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_SEMANTIC;
	semanticJudge = new CSemanticJudge(this);
	cmpword = new CCmpWord(this);
//	penreader = new CPenReader;
	mysql = new CMysqlHandler();
	chihlee = new CChihlee();
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
	if (strConfPath.empty())
		return nResult;

	config = new CConfig();
	if (config->loadConfig(strConfPath))
	{
		strPort = config->getValue("SERVER WORD", "port");
		if (!strPort.empty())
		{
			convertFromString(nPort, strPort);
			nResult = cmpword->start(0, nPort, mnMsqKey);
		}
	}
	delete config;

	semanticJudge->loadAnalysis();

	mysql->connect("127.0.0.1", "edubot", "edubot", "ideas123!", "5");

	return nResult;
}

int CController::onFinish(void* nMsqKey)
{
	mysql->close();
	cmpword->stop();
	delete cmpword;
	delete semanticJudge;
//	delete penreader;
	delete mysql;

	return TRUE;
}

void CController::onSemanticWordRequest(const int nSocketFD, const int nSequence, const int nId, const int nType,
		const char *szWord)
{
	CString strWord;
	CString strDevice_id;
	JSONObject jsonReq;
	JSONObject jsonResp;
	jsonResp.create();

	jsonReq.load(szWord);
	strWord = jsonReq.getString("word");
	strDevice_id = jsonReq.getString("device_id");
	jsonReq.release();

	//=================== chihlee start===================================//
	_log("device id: %s ===============================", strDevice_id.getBuffer());
	if (0 == strDevice_id.Compare("chihlee")) // 致理科大導覽系統
	{
		chihlee->runAnalysis(strWord.getBuffer(), jsonResp);
		cmpword->response(nSocketFD, semantic_word_request, STATUS_ROK, nSequence,
				jsonResp.put("id", nId).toJSON().c_str());
		recordResponse(strDevice_id.getBuffer(), 0, nType, jsonResp.toJSON().c_str());
		jsonResp.release();
		return;
	}
	//======================= chihlee end ===================================//

	_log("[CController] onSemanticWordRequest device_id: %s word: %s", strDevice_id.getBuffer(), strWord.getBuffer());

	if (!strWord.Compare("分析垃圾"))
	{
		cmpword->response(nSocketFD, semantic_word_request, STATUS_ROK, nSequence,
				jsonResp.put("id", nId).toJSON().c_str());
		CStory *story = new CStory;
		story->storyAnalysis(STORY_FILE_PATH);
		delete story;
		return;
	}

	if (!strWord.Compare("emotion"))
	{
		cmpword->response(nSocketFD, semantic_word_request, STATUS_ROK, nSequence,
				jsonResp.put("id", nId).toJSON().c_str());
		CParticiple *participle = new CParticiple;
		set<string> smark;
		smark.insert("。");
		participle->splitter(STORY_FILE_PATH, "。");
		delete participle;
		return;
	}

	switch (nType)
	{
	case TYPE_REQ_NODEFINE: // 語意判斷
		semanticJudge->runAnalysis(strWord.getBuffer(), jsonResp);
		break;
	case TYPE_REQ_CONTROL:	// 控制
		break;
	case TYPE_REQ_TALK: 	// 會話
		break;
	case TYPE_REQ_RECORD:	// 紀錄
		break;
	case TYPE_REQ_STORY:	// 故事
		semanticJudge->runAnalysis(strWord.getBuffer(), jsonResp, "story");
		break;
	case TYPE_REQ_GAME:		// 遊戲
		break;
	case TYPE_REQ_PEN:		// 點讀筆
//		penreader->activity(strWord.getBuffer(), jsonResp);
		break;
	default:
		cmpword->response(nSocketFD, semantic_word_request, STATUS_RINVJSON, nSequence, 0);
		jsonResp.release();
		return;
	}
	cmpword->response(nSocketFD, semantic_word_request, STATUS_ROK, nSequence,
			jsonResp.put("id", nId).toJSON().c_str());
	recordResponse(strDevice_id.getBuffer(), 0, nType, jsonResp.toJSON().c_str());
	jsonResp.release();
}

void CController::recordResponse(const char * szDevice_id, int nSemantic_id, int nType, const char * szData)
{
	CString strSQL;

	if (!mysql->isValid())
	{
		if (!mysql->connect("127.0.0.1", "edubot", "edubot", "ideas123!", "5"))
		{
			_log("[CController] recordResponse mysql invalid, can't record response: %s", szData);
			return;
		}
	}

	strSQL.format("INSERT INTO response (device_id, semantic_id, type, response) VALUES ('%s', %d, %d,'%s')",
			szDevice_id, nSemantic_id, nType, szData);
	mysql->sqlExec(strSQL.getBuffer());
}

void CController::onHandleMessage(Message &message)
{
	switch (message.what)
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
