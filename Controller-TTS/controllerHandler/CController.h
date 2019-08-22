/*
 * CController.h
 *
 *  Created on: 2018年9月27日
 *      Author: Jugo
 *
 */

#pragma once

#include "CApplication.h"
#include <vector>

class CTextProcess;
class CCmpTTS;

using namespace std;

typedef struct _TTS_REQ
{
	std::string user_id;
	int voice_id;
	int emotion;
	std::string text;
	std::string fm;
	std::string g;
	std::string r;
	std::string id;
	int total;
	int sequence_num;
	int req_type;
} TTS_REQ;

class CController: public CApplication
{
public:
	CController();
	virtual ~CController();
	void onTTS(const int nSocketFD, const int nSequence, const char *szData);
	vector<string> splitSentence(string &input);
	bool checkEnglish(string &input);


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
