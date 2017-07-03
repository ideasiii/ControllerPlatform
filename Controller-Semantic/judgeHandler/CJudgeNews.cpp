/*
 * CJudgeNews.cpp
 *
 *  Created on: 2017年6月16日
 *      Author: Jugo
 */

#include "CJudgeNews.h"
#include "common.h"
#include "CResponsePacket.h"

using namespace std;

CJudgeNews::CJudgeNews()
{

}

CJudgeNews::~CJudgeNews()
{

}

string CJudgeNews::toString()
{
	return "CJudgeNews";
}

int CJudgeNews::word(const char *szInput, JSONObject& jsonResp, std::map<std::string, std::string> &mapMatch)
{
	return TRUE;
}

int CJudgeNews::evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch)
{
	return TRUE;
}

void CJudgeNews::loadNewsDictionary()
{

}

