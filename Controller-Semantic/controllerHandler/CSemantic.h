/*
 * CSemantic.h
 *
 *  Created on: 2017年5月2日
 *      Author: Jugo
 *      .....幹! 亂七八遭
 */

#pragma once

#include <map>
#include <set>

class JSONObject;

typedef struct _WORD_ATTR
{
	int nIndex;				// 字詞位置
	int nAttribute;			// 字詞屬性
	int nSubAttr;			// 字詞副屬性
	std::string strWord;	// 字詞
	_WORD_ATTR()
	{
		nIndex = -1;
		nAttribute = -1;
		nSubAttr = -1;
	}
} WORD_ATTR;

typedef std::map<int, WORD_ATTR> WORD_BODY;		// 文字的肉體

class CSemantic
{
public:
	virtual ~CSemantic();
	int getSubject(const char *szWord);
	int getAttribute(const char *szWord, WORD_BODY &wordBody);
	int getVerb(const char *szWord, WORD_ATTR &wordAttr);
	int _evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);
	int _word(const char *szInput, JSONObject* jsonResp, std::map<std::string, std::string> &mapMatch);
	std::string _toString();

protected:
	virtual std::string toString();
	virtual int word(const char *szInput, JSONObject* jsonResp, std::map<std::string, std::string> &mapMatch);
	virtual int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);

};
