/*
 * CSemanticJudge.h
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"

class JSONObject;
//class CObject;
class CSemantic;
class CAnalysisHandler;

class CSemanticJudge: public CObject
{

public:
	explicit CSemanticJudge(CObject *object);
	virtual ~CSemanticJudge();
//	int word(const char *szInput, JSONObject& jsonResp);
	void runAnalysis(const char *szInput, JSONObject &jsonResp);
	void runAnalysis(const char *szInput, JSONObject &jsonResp, const char *szAnalysis);
	void loadAnalysis();

protected:
	void onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
	{
	}
	;
private:
	CObject *mpController;
//	std::map<int, CSemantic*> mapSemanticObject;
	std::map<int, CAnalysisHandler*> mapAnalysis;
};
