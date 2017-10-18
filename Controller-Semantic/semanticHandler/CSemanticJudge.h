/*
 * CSemanticJudge.h
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"

class JSONObject;
class CSemantic;
class CAnalysisHandler;
class CSemanticService;

class CSemanticJudge: public CObject
{

public:
	explicit CSemanticJudge(CObject *object);
	virtual ~CSemanticJudge();
	void runAnalysis(const char *szInput, JSONObject &jsonResp);
	void runAnalysis(const char *szInput, JSONObject &jsonResp, const char *szAnalysis);
	void loadAnalysis();

protected:
	void onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
	{
	}
	;

	void onHandleMessage(Message &message);
private:
	CObject *mpController;
	std::map<int, CSemanticService*> mapSemanticService;
	std::map<int, CAnalysisHandler*> mapAnalysis;
};
