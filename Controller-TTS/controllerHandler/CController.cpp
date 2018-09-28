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
#include "event.h"
#include "CTextProcess.h"

using namespace std;

CController::CController() :
		mnMsqKey(0), textProcess(0)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_TTS;
	textProcess = new CTextProcess();
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	string strConfPath = reinterpret_cast<const char*>(szConfPath);

	// test
	textProcess->processTheText("哈哈哈，嘻嘻嘻。喔喔喔喔喔!\n嗚嗚嗚嗚嗚嗚。");
	return 0;
}

int CController::onFinish(void* nMsqKey)
{
	int nKey = *(reinterpret_cast<int*>(nMsqKey));
	delete textProcess;
	return nKey;
}

void CController::onHandleMessage(Message &message)
{
	switch(message.what)
	{

	}
}
