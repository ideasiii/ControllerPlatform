/*
 * CTextProcess.cpp
 *
 *  Created on: 2018年9月28日
 *      Author: Jugo
 */

#include "CTextProcess.h"
#include "common.h"
#include "CString.h"
#include <vector>

using namespace std;

CTextProcess::CTextProcess()
{

}

CTextProcess::~CTextProcess()
{

}

void CTextProcess::processTheText(const char *szText)
{
	int nIndex;
	int nLen = 0;
	int nCount = 0;
	CString strInput;
	CString strPart;

	if(!szText)
		return;

	strInput = szText;
	_log("[CTextProcess] processTheText Input Text: %s", strInput.getBuffer());
	// 斷句. 先把input的文章存成sentence Array
	int i, j, k, l, lcount, sIndex, wIndex, pIndex;
	vector<CString> SentenceArray;
	CString strTemp1, strResult;
	strTemp1 = strInput.SpanExcluding("\n");
	strTemp1 = strTemp1.SpanExcluding("\r");
	strResult = strTemp1;
	_log("[CTextProcess] processTheText SpanExcluding: %s", strResult.getBuffer());

	/**
	 * 全形符號斷詞 。？！；，
	 */
	vector<string> vs = {"。", "？", "！", "；", "，","!",",",".",";","?"};
	string strFinded;
	while(strResult.findOneOf(vs,strFinded) != -1)
	{
		CString temp;
		i = strResult.findOneOf(vs,strFinded);
		temp = strResult.left(i);
		strResult = strResult.right(strResult.getLength() - i - strFinded.length());
		SentenceArray.push_back(temp);
		_log("remain: %s finded: %s", strResult.getBuffer(),strFinded.c_str());
	}



	for(vector<CString>::iterator it = SentenceArray.begin(); SentenceArray.end() != it; ++it)
	{
		_log("[CTextProcess] processTheText Sentence: %s", it->getBuffer());
	}
}

