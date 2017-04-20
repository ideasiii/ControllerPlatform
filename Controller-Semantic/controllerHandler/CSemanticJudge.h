/*
 * CSemanticJudge.h
 *
 *  Created on: 2017年4月19日
 *      Author: Jugo
 */

#pragma once

#define MAX_WORD_ATTR		7

class JSONObject;

class CSemanticJudge
{
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

public:
	explicit CSemanticJudge();
	virtual ~CSemanticJudge();
	void word(const char *szInput, JSONObject* jsonResp);

private:
	int getSubject(const char *szWord);
	int getAttribute(const char *szWord, WORD_BODY &wordBody);
};
