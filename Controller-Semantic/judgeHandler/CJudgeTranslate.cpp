/*
 * CJudgeTranslate.cpp
 *
 *  Created on: 2017年7月3日
 *      Author: jugo
 */

#include <set>
#include <string>
#include "CJudgeTranslate.h"
#include "common.h"
#include "config.h"
#include "CTranslate.h"
#include "CResponsePacket.h"

using namespace std;

CJudgeTranslate::CJudgeTranslate()
{

}

CJudgeTranslate::~CJudgeTranslate()
{

}

string CJudgeTranslate::toString()
{
	return "CJudgeTranslate";
}
int CJudgeTranslate::word(const char *szInput, JSONObject& jsonResp, std::map<std::string, std::string> &mapMatch)
{
	string strWord;
	string strTranslate;
	CTranslate translate;
	CResponsePacket respPacket;
	RESULT result;

	strWord = szInput;
	if(strWord.empty())
		return FALSE;

	translate.translate(en, strWord.c_str(), result);
	strWord = result.strResult;

	respPacket.setData("lang", "en").setData("content", strWord).format(TYPE_RESP_TTS, jsonResp);

	return 0;
}

int CJudgeTranslate::evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch)
{
	int nScore;
	string strWord;
	string strLocation;
	map<string, int>::const_iterator it_map;
	set<string>::const_iterator it_set;

	nScore = 0;
	strWord = szWord;

	if(strWord.empty())
		return 0;

	//======== 評估字典檔 ==========//
	if(string::npos != strWord.find("翻譯"))
	{
		++nScore;
	}

	return nScore;
}

