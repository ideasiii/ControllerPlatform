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
class CJudgeStory;
class CJudgeMusic;

class CSemanticJudge
{
public:
	explicit CSemanticJudge(CObject *object);
	virtual ~CSemanticJudge();
	int word(const char *szInput, JSONObject* jsonResp);

private:
	CObject *mpController;
	CJudgeStory *mpJudgeStory;
	CJudgeMusic *mpJudgeMusic;
	std::map<int, CSemantic*> mapSemanticObject;
};
