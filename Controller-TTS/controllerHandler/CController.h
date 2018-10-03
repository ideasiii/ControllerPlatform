/*
 * CController.h
 *
 *  Created on: 2018年9月27日
 *      Author: Jugo
 *
 */

#pragma once

#include "CApplication.h"

class CTextProcess;
class CCmpTTS;

typedef struct _TTS_REQ
{
	std::string user_id;
	int voice_id;
	int emotion;
	std::string text;
} TTS_REQ;

class CController: public CApplication
{
public:
	CController();
	virtual ~CController();
	void onTTS(const int nSocketFD, const int nSequence, const char *szData);

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);

private:
	int mnMsqKey;
	CTextProcess *textProcess;
	CCmpTTS *cmpTTS;
};
