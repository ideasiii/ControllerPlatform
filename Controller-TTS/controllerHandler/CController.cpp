/*
 * CController.cpp
 *
 *  Created on: 2018年9月27日
 *      Author: Jugo
 */
#include "CController.h"

#include <stdio.h>
#include <string>
#include <typeinfo>
#include <iostream>
#include <thread>
#include "common.h"
#include "event.h"
#include "CTextProcess.h"
#include "CCmpTTS.h"
#include "CConfig.h"
#include "utility.h"
#include "packet.h"
#include "JSONObject.h"

using namespace std;

CController::CController() :
		mnMsqKey(0), textProcess(0), cmpTTS(0)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_TTS;
	textProcess = new CTextProcess();
	cmpTTS = new CCmpTTS(this);
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nResult = 0;
	int nCount;
	int nPort;
	string strConfPath;
	string strPort;
	CConfig *config;

	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if(!strConfPath.empty())
	{
		config = new CConfig();
		if(config->loadConfig(strConfPath))
		{
			strPort = config->getValue("SERVER", "port");
			if(!strPort.empty())
			{
				convertFromString(nPort, strPort);
				nResult = cmpTTS->start(0, nPort, mnMsqKey);
			}
		}
		delete config;
	}
	// test
	textProcess->processTheText("憋魚酒氣了");
	return nResult;
}

int CController::onFinish(void* nMsqKey)
{
	int nKey = *(reinterpret_cast<int*>(nMsqKey));
	delete textProcess;
	delete cmpTTS;
	return nKey;
}

/*
 * "user_id":"",
 "voice_id":0,
 "emotion":0,
 "text":"多型態角色語音智慧平台"
 */
void CController::onTTS(const int nSocketFD, const int nSequence, const char *szData)
{
	JSONObject jsonReq;
	JSONObject jsonResp;
	TTS_REQ ttsReq;

	jsonReq.load(szData);
	ttsReq.user_id = jsonReq.getString("user_id");
	ttsReq.voice_id = jsonReq.getInt("voice_id");
	ttsReq.emotion = jsonReq.getInt("emotion");
	ttsReq.text = jsonReq.getString("text");
	jsonReq.release();
	_log("[CController] onTTS socketFD: %d text: %s user: %s voice: %d emotion: %d", nSocketFD, ttsReq.text.c_str(),
			ttsReq.user_id.c_str(), ttsReq.voice_id, ttsReq.emotion);
	textProcess->processTheText(ttsReq.text.c_str());

	jsonResp.create();
	jsonResp.put("status", 0);
	jsonResp.put("wave", "http://54.199.198.94/tts/Wate.wav");
	cmpTTS->response(nSocketFD, tts_request, STATUS_ROK, nSequence, jsonResp.toJSON().c_str());
	jsonResp.release();
}

void CController::onHandleMessage(Message &message)
{
	switch(message.what)
	{
	case tts_request:
		thread([=]
		{	onTTS( message.arg[0], message.arg[1],message.strData.c_str());}).detach();
		break;

	}
}
