/*
 * CSemanticJudge.h
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#pragma once

class JSONObject;
class CObject;
class CSemantic;
class CAnalysisHandler;

class CSemanticJudge
{

public:
	explicit CSemanticJudge(CObject *object);
	virtual ~CSemanticJudge();
	int word(const char *szInput, JSONObject& jsonResp);
	void runAnalysis(const char *szInput, JSONObject &jsonResp);
	void runAnalysis(const char *szInput, JSONObject &jsonResp, const char *szAnalysis);
	void loadAnalysis();

private:
	CObject *mpController;
	std::map<int, CSemantic*> mapSemanticObject;
	std::map<int, CAnalysisHandler*> mapAnalysis;
};
