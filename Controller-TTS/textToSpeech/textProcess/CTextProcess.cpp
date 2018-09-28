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
	CString strInput;
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

	while(strResult.FindOneOf("。？！；，") != -1)
	{
		CString temp;
		i = strResult.FindOneOf("。？！；，");
		temp = strResult.left(i);
		strResult = strResult.right(strResult.getLength() - i - 2);
		SentenceArray.push_back(temp);
	}

	for(vector<CString>::iterator it = SentenceArray.begin(); SentenceArray.end() != it; ++it)
	{
		_log("[CTextProcess] processTheText Sentence: %s", it->getBuffer());
	}
}

