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
	vector<string> vs = {"。", "？", "！", "；", "，"};
	while(strResult.findOneOf(vs) != -1)
	{
		CString temp;
		i = strResult.findOneOf(vs);
		temp = strResult.left(i);
		strResult = strResult.right(strResult.getLength() - i - 3);
		SentenceArray.push_back(temp);
	}

	/*
	 nIndex = 0;
	 string strText = strResult.getBuffer();
	 string strDot[5] = { "。", "？", "！", "；", "，" };
	 string strDotCur;
	 while(1)
	 {
	 for(int i = 0; i < 5; ++i)
	 {
	 _log("[CTextProcess] 全形符號: %s 斷詞", strDot[i].c_str());
	 if(nCount)
	 nIndex = nIndex + nLen + strDot[i].length(); //strlen("。");

	 nLen = strText.find(strDot[i], nIndex);
	 if((int) string::npos == nLen)
	 {
	 _log("no found %s..................", strDot[i].c_str());
	 }
	 else
	 {
	 nLen = nLen - nIndex;
	 printf("index=%d , size=%d\n", nIndex, nLen);
	 strPart = strText.substr(nIndex, nLen);
	 printf("part: %s\n", strPart.getBuffer());
	 ++nCount;
	 }
	 }
	 if(nIndex >= strText.length())
	 break;
	 }
	 */

	for(vector<CString>::iterator it = SentenceArray.begin(); SentenceArray.end() != it; ++it)
	{
		_log("[CTextProcess] processTheText Sentence: %s", it->getBuffer());
	}
}

