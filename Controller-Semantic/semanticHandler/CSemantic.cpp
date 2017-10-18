/*
 * CSemantic.cpp
 *
 *  Created on: 2017年5月2日
 *      Author: Jugo
 */

#include <map>
#include <string>
#include "CSemantic.h"
#include "config.h"
#include "JSONObject.h"

using namespace std;

#define MAX_WORD_ATTR		7

extern map<string, int> mapSubject;

CSemantic::~CSemantic()
{

}

int CSemantic::getVerb(const char *szWord, WORD_ATTR &wordAttr)
{
	int nIndex = -1;
	map<string, int>::iterator it;
	string strWord = szWord;
	extern map<string, int> mapVerb;

	for(it = mapVerb.begin(); it != mapVerb.end(); ++it)
	{
		nIndex = strWord.find(it->first);
		if((int) string::npos != nIndex)
		{
			wordAttr.nAttribute = VERB;
			wordAttr.nIndex = nIndex;
			wordAttr.nSubAttr = it->second;
			wordAttr.strWord = it->first;
			break;
		}
	}

	return nIndex;
}

int CSemantic::getSubject(const char *szWord)
{
	extern map<string, int> mapSubject;

	if(0 < szWord)
	{
		string strWord = szWord;
		map<string, int>::iterator it;

		for(it = mapSubject.begin(); it != mapSubject.end(); ++it)
		{
			if(string::npos != strWord.find(it->first))
				return it->second;
		}
	}

	return 0;
}

int CSemantic::getAttribute(const char *szWord, WORD_BODY &wordBody)
{
	int nCount;
	int nNumber;
	int nIndex;
	string strWord = szWord;
	map<string, int>::iterator it;

	// 主詞
	nNumber = 0;
	nCount = 0;
	for(it = mapSubject.begin(); it != mapSubject.end(); ++it)
	{
		nIndex = strWord.find(it->first);
		if((int) string::npos != nIndex)
		{
			nNumber = nCount++;
			if(MAX_WORD_ATTR <= nNumber)
				break;
			WORD_ATTR wordAttr;
			wordAttr.nAttribute = SUBJECT;
			wordAttr.nIndex = nIndex;
			wordAttr.nSubAttr = it->second;
			wordAttr.strWord = it->first;
			wordBody[nNumber] = wordAttr;
		}
	}
	return 0;
}

//int CSemantic::_word(const char *szInput, JSONObject& jsonResp, map<string, string> &mapMatch)
//{
//	return word(szInput, jsonResp, mapMatch);
//}

//int CSemantic::word(const char *szInput, JSONObject& jsonResp, map<string, string> &mapMatch)
//{
//	return 0;
//}

//int CSemantic::_evaluate(const char *szWord, map<string, string> &mapMatch)
//{
//	return evaluate(szWord, mapMatch);
//}

//int CSemantic::evaluate(const char *szWord, map<string, string> &mapMatch)
//{
//	return 0;
//}

//string CSemantic::_toString()
//{
//	return toString();
//}

//string CSemantic::toString()
//{
//	return "CSemantic";
//}

