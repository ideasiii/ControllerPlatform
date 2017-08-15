/*
 * CController.h
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CSemanticJudge;
class CCmpWord;
class CPenReader;

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();
	void onSemanticWordRequest(const int nSocketFD, const int nSequence, const int nId, const int nType,
			const char *szWord);

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);

private:
	int mnMsqKey;
	CCmpWord *cmpword;
	CSemanticJudge *semanticJudge;
	CPenReader *penreader;
};
