/*
 * CController.h
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CSemanticJudge;
class CSemanticControl;
class CSemanticTalk;
class CSemanticRecord;
class CCmpWord;

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);

private:
	void onSemanticWordRequest(const int nSocketFD, const int nSequence, const int nId, const int nType,
			const char *szWord);

private:
	int mnMsqKey;
	CCmpWord *cmpword;
	CSemanticJudge *semanticJudge;
	CSemanticControl *semanticControl;
	CSemanticTalk *semanticTalk;
	CSemanticRecord *semanticRecord;
};
