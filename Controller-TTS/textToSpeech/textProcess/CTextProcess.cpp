/*
 * CTextProcess.cpp
 *
 *  Created on: 2018年9月28日
 *      Author: Jugo
 */

#include <climits>
#include "CTextProcess.h"
#include "common.h"
#include "CString.h"
#include "CStringArray.h"

using namespace std;

CTextProcess::CTextProcess() :
		gduration_s(0), gduration_e(0), giSftIdx(0)
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
	CStringArray SentenceArray;
	CString strTemp1, strResult;
	strTemp1 = strInput.SpanExcluding("\n");
	strTemp1 = strTemp1.SpanExcluding("\r");
	strResult = strTemp1;
	_log("[CTextProcess] processTheText SpanExcluding: %s", strResult.getBuffer());

	/**
	 * 全形符號斷詞 。？！；，與半形符號斷詞
	 */
	vector<string> vs = { "。", "？", "！", "；", "，", "!", ",", ".", ";", "?" };
	string strFinded;
	while(strResult.findOneOf(vs, strFinded) != -1)
	{
		CString temp;
		i = strResult.findOneOf(vs, strFinded);
		temp = strResult.left(i);
		strResult = strResult.right(strResult.getLength() - i - strFinded.length());
		SentenceArray.add(temp);
		_log("Text: %s finded: %s", temp.getBuffer(), strFinded.c_str());
	}

	for(i = 0; i < SentenceArray.getSize(); ++i)
	{
		_log("[CTextProcess] processTheText Sentence: %s", SentenceArray[i].getBuffer());
	}

	/*********************************************************************
	 *   以sentence為單位合成
	 *********************************************************************/
	char s[10];
	int SyllableBound[100];		// syllable邊界
	int SyllableTone[100];		// tone of syllable
	int WordBound[100];			// word boundary, tts 斷詞結果
	int PhraseBound[20];		// phrase boundary
	SyllableTone[0] = 0;
	gduration_s = NULL;
	gduration_e = NULL;
	giSftIdx = 0;
	int playcount = 0;

	for(lcount = 0; lcount < (int) SentenceArray.getSize(); ++lcount)
	{
		CStringArray PhoneSeq;	// 紀錄整個utterance的phone model sequence  音節  音素
		CString strBig5;
		vector<int> PWCluster;
		vector<int> PPCluster;

		// initial boundaries and indexes for a sentence
		sIndex = wIndex = pIndex = 0;
		SyllableBound[0] = WordBound[0] = PhraseBound[0] = -1;
		for(i = 1; i < 100; i++)
			SyllableBound[i] = WordBound[i] = 0;
		for(i = 1; i < 20; i++)
			PhraseBound[i] = 0;

		// 簡單處理標點符號及分隔字符
		SentenceArray[lcount].replace(" ", "");
		SentenceArray[lcount].replace("\t", "");
		vector<string> vs1 = { "：", "、", "（", "）", "「", "」" };
		for(i = SentenceArray[lcount].findOneOf(vs1, strFinded); i != -1;
				i = SentenceArray[lcount].findOneOf(vs1, strFinded))
			SentenceArray[lcount].Delete(i, strFinded.length());
		vector<string> vs2 = { ":", ";", "?", "!", "(", ")", "[", "]" };
		for(i = SentenceArray[lcount].findOneOf(vs2, strFinded); i != -1;
				i = SentenceArray[lcount].findOneOf(vs2, strFinded))
			SentenceArray[lcount].Delete(i, strFinded.length());

		CartPrediction(SentenceArray[lcount], strBig5, PWCluster, PPCluster);
	}
}

void CTextProcess::CartPrediction(CString &sentence, CString &strBig5, vector<int>& allPWCluster,
		vector<int>& allPPCluster)
{
	int featureDim = 14;
	int nCluster = 2;
	vector<int> wordpar;	// 當前的詞長
	vector<int> syllpos;	// 當前的字在詞中位置
	vector<int> cluster;	// 韻律詞結尾=1, otherwise 0
	vector<int> pos;		// first: 幾字詞, second: POS tagging
	vector<int> pwBoundary;
}

void CTextProcess::GenerateLabelFile(CStringArray& sequence, const int sBound[], const int wBound[], const int pBound[],
		const int sCount, const int wCount, const int pCount, ofstream& csFile, ofstream *pcsFile2)
{
	CString fullstr, tempstr; // fullstr: store all lines for full labels
	CString monostr; // ky add: store all lines for mono labels
	int sIndex, wIndex, pIndex; // syllable/word/phrase index for sBound/wBound/pBound
	sIndex = wIndex = pIndex = 0;
	fullstr = "";
	monostr = "";

	// ky add: output time information from cue. use timeinfo() to enable "outpu time information"
	char timebuf[25]; // tag of time. calculated from cue and syllabel boundaries
	if(gduration_s != NULL) // output time info for the first pause label
	{
		int tmp = 0;
		if(gduration_s[1] - 1000000 > 0)
			tmp = gduration_s[1] - 1000000;
		tempstr.format("%10d %10d ", tmp, gduration_s[1]);
		fullstr += tempstr;
		monostr += tempstr;
	}
	//

	// p1^p2-p3+p4=p5@p6_p7/A:a3/B:b3@b4-b5&b6-b7/C:c3/D:d2/E:e2@e3+e4
	// /F:f2/G:g1_g2/H:h1=h2@h3=h4/I:i1=i2/J:j1+j2-j3
	tempstr.format("x^x-pau+%s=%s@x_x/A:0/B:x@x-x&x-x/C:%d/D:0/E:x@x+x", sequence[0], sequence[1], sBound[1] + 1);// current phoneme = pause (pau);
	fullstr += tempstr; // ky modify: don't initial fullstr, since I'll do it myself.
	tempstr.format("pau\n"); // ky add: for mono
	monostr += tempstr; // ky add: for mono
	int anchor, anchor2;
	anchor = anchor2 = 0;
	while(sBound[anchor] != wBound[1]) 	// f2
		anchor++;
	tempstr.format("/F:%d/G:0_0/H:x=x@1=%d", anchor, pCount);
	fullstr += tempstr;
	if(pCount == 1)	// i1, i2
		tempstr.format("/I:%d=%d", sCount, wCount);
	else
	{
		anchor = 0;
		while(sBound[anchor] != pBound[1])	// i1
			anchor++;
		tempstr.format("/I:%d", anchor);
		fullstr += tempstr;
		anchor = 0;
		while(wBound[anchor] != pBound[1])	// i2
			anchor++;
		tempstr.format("=%d", anchor);
		fullstr += tempstr;
	}
	tempstr.format("/J:%d+%d-%d\n", sCount, wCount, pCount);
	fullstr += tempstr;
	int iMM = INT_MAX; //index mm: syllable_phone   phone index. since the mm is assigned from sylla_p[ll], the value is initialed as maximum value, and reassigned in the loop
	for(int index = 0; index < sequence.getSize(); index++)	// index = current phone
	{
		if(sBound[sIndex] < index)
			sIndex++;
		if(wBound[wIndex] < index)
			wIndex++;
		if(pBound[pIndex] < index)
			pIndex++;

		// ky add: add time info for each line of label.
		if(gduration_s != NULL)
		{
			// simulate indexes in old version: window_synthesis_demo: textpross::labelgen()
			int iLL = sIndex - 1;				// index ll: word_syllable   syllable index
			int iSy_p_ll = sBound[iLL] + 1;			// sylla_p[ll]
			int iSy_p_ll_1 = sBound[iLL + 1] + 1;		// sylla_p[ll+1]
			if(iMM >= iSy_p_ll_1 + 2)			// index mm: syllable_phone   phone index. simulate the loop of index mm
				iMM = iSy_p_ll + 2;
			else
				iMM++;

			// output time info
			tempstr.format("%10d %10d ",
					gduration_s[iLL + 1]
							+ (gduration_e[iLL + 1] - gduration_s[iLL + 1]) * (iMM - iSy_p_ll - 2)
									/ (iSy_p_ll_1 - iSy_p_ll),
					gduration_s[iLL + 1]
							+ (gduration_e[iLL + 1] - gduration_s[iLL + 1]) * (iMM - iSy_p_ll - 1)
									/ (iSy_p_ll_1 - iSy_p_ll));
			fullstr += tempstr;
			monostr += tempstr;
			tempstr.format("%s\n", sequence[index]);
			monostr += tempstr;
		}
		//

		// p1~p5
		if(index < 2)
		{
			if(index == 0)
			{
				if(sequence.getSize() == 1)
					tempstr.format("x^pau-%s+x=x", sequence[index]);
				else if(sequence.getSize() == 2)
					tempstr.format("x^pau-%s+%s=x", sequence[index], sequence[index + 1]);
				else
					tempstr.format("x^pau-%s+%s=%s", sequence[index], sequence[index + 1], sequence[index + 2]);
			}
			else	// index == 1
			{
				if(sequence.getSize() == 2)
					tempstr.format("pau^%s-%s+x=x", sequence[index - 1], sequence[index]);
				else if(sequence.getSize() == 3)
					tempstr.format("pau^%s-%s+%s=x", sequence[index - 1], sequence[index], sequence[index + 1]);
				else
					tempstr.format("pau^%s-%s+%s=%s", sequence[index - 1], sequence[index], sequence[index + 1],
							sequence[index + 2]);
			}
		}
		else if(index > sequence.getSize() - 3)
		{
			if(index == sequence.getSize() - 2)
				tempstr.format("%s^%s-%s+%s=pau", sequence[index - 2], sequence[index - 1], sequence[index],
						sequence[index + 1]);
			else
				tempstr.format("%s^%s-%s+pau=x", sequence[index - 2], sequence[index - 1], sequence[index]);
		}
		else
			tempstr.format("%s^%s-%s+%s=%s", sequence[index - 2], sequence[index - 1], sequence[index],
					sequence[index + 1], sequence[index + 2]);
		fullstr += tempstr;

		// p6, p7
		tempstr.format("@%d_%d", index - sBound[sIndex - 1], sBound[sIndex] + 1 - index);
		fullstr += tempstr;

		// a3, b3
		if(sIndex == 1)
			tempstr.format("/A:0/B:%d", sBound[sIndex] - sBound[sIndex - 1]);
		else
			tempstr.format("/A:%d/B:%d", sBound[sIndex - 1] - sBound[sIndex - 2], sBound[sIndex] - sBound[sIndex - 1]);
		fullstr += tempstr;

		// b4, b5
		anchor = sIndex;
		while(wBound[wIndex - 1] < sBound[anchor])
			anchor--;
		tempstr.format("@%d", sIndex - anchor);
		fullstr += tempstr;
		anchor = sIndex;
		while(wBound[wIndex] > sBound[anchor])
			anchor++;
		tempstr.format("-%d", anchor - sIndex + 1);
		fullstr += tempstr;

		// b6, b7
		anchor = sIndex;
		while(pBound[pIndex - 1] < sBound[anchor])
			anchor--;
		tempstr.format("&%d", sIndex - anchor);
		fullstr += tempstr;
		anchor = sIndex;
		while(pBound[pIndex] > sBound[anchor])
			anchor++;
		tempstr.format("-%d", anchor - sIndex + 1);
		fullstr += tempstr;

		// c3
		if(sIndex == sCount)
			tempstr.format("/C:0");
		else
			tempstr.format("/C:%d", sBound[sIndex + 1] - sBound[sIndex]);
		fullstr += tempstr;

		// d2
		if(wIndex == 1)
			tempstr.format("/D:0");
		else
		{
			anchor = sIndex;
			while(sBound[anchor] != wBound[wIndex - 1])
				anchor--;
			anchor2 = anchor;
			while(wBound[wIndex - 2] < sBound[anchor2])
				anchor2--;
			tempstr.format("/D:%d", anchor - anchor2);
		}
		fullstr += tempstr;
		// e2
		anchor = sIndex;
		while(wBound[wIndex - 1] < sBound[anchor])
			anchor--;
		anchor2 = sIndex;
		while(wBound[wIndex] > sBound[anchor2])
			anchor2++;
		tempstr.format("/E:%d", anchor2 - anchor);
		fullstr += tempstr;

		// e3, e4
		anchor = wIndex;
		while(pBound[pIndex - 1] < wBound[anchor])
			anchor--;
		tempstr.format("@%d", wIndex - anchor);
		fullstr += tempstr;
		anchor = wIndex;
		while(pBound[pIndex] > wBound[anchor])
			anchor++;
		tempstr.format("+%d", anchor - wIndex + 1);
		fullstr += tempstr;

		// f2:  #of syllable in the next next word
		anchor = sIndex;
		while(sBound[anchor] < wBound[wIndex])
			anchor++;
		anchor2 = anchor;	// anchor2: where the next word start
		while(sBound[anchor] < wBound[wIndex + 1])
			anchor++;
		tempstr.format("/F:%d", anchor - anchor2);
		fullstr += tempstr;

		// g1:	#of syllables in the previous phrase
		// g2:	#of words in the previous phrase
		if(pIndex == 1)
			tempstr.format("/G:0_0");
		else
		{
			anchor = sIndex;
			while(sBound[anchor] > pBound[pIndex - 1])
				anchor--;
			anchor2 = anchor;
			while(pBound[pIndex - 2] < sBound[anchor2])
				anchor2--;
			tempstr.format("/G:%d", anchor - anchor2);
			fullstr += tempstr;
			anchor = wIndex;
			while(wBound[anchor] > pBound[pIndex - 1])
				anchor--;
			anchor2 = anchor;
			while(pBound[pIndex - 2] < wBound[anchor2])
				anchor2--;
			tempstr.format("_%d", anchor - anchor2);
		}
		fullstr += tempstr;

		// h1:	#of syllables in the current phrase
		// h2:	#of words in the current phrase
		anchor = anchor2 = sIndex;
		while(pBound[pIndex - 1] < sBound[anchor])
			anchor--;
		while(pBound[pIndex] > sBound[anchor2])
			anchor2++;
		tempstr.format("/H:%d", anchor2 - anchor);
		fullstr += tempstr;
		anchor = anchor2 = wIndex;
		while(pBound[pIndex - 1] < wBound[anchor])
			anchor--;
		while(pBound[pIndex] > wBound[anchor2])
			anchor2++;
		tempstr.format("=%d", anchor2 - anchor);
		fullstr += tempstr;

		tempstr.format("@%d=%d", pIndex, pCount - pIndex + 1);	// h3, h4
		fullstr += tempstr;
		// i1, i2
		if(pCount == 1)
			tempstr.format("/I:0=0");
		else
		{
			anchor = anchor2 = sIndex;
			while(pBound[pIndex] > sBound[anchor])
				anchor++;
			anchor2 = anchor;
			while(pBound[pIndex + 1] > sBound[anchor])
				anchor++;
			tempstr.format("/I:%d", anchor - anchor2);
			fullstr += tempstr;
			anchor = anchor2 = wIndex;
			while(pBound[pIndex] > wBound[anchor])
				anchor++;
			anchor2 = anchor;
			while(pBound[pIndex + 1] > wBound[anchor])
				anchor++;
			tempstr.format("=%d", anchor - anchor2);
			fullstr += tempstr;
		}
		tempstr.format("/J:%d+%d-%d\n", sCount, wCount, pCount);	// j1,j2,j3
		fullstr += tempstr;
	}
	//	index--;
	int index = sequence.getSize() - 1;		//20091211 rosy edit for .Net 取代上面那行 index --

	// ky add: add time info for pau at the end of the sentence
	if(gduration_s != NULL)
	{
		int tmp = 0;
		int iCount_2 = sCount;  // simulate index for count[2]
		giSftIdx += sCount;  // update global shift of syllable index for each sentence
		tmp = gduration_e[iCount_2] + 1000000;
		tempstr.format("%10d %10d ", gduration_e[iCount_2], tmp);
		fullstr += tempstr;
		monostr += tempstr;
	}
	//

	tempstr.format("%s^%s-pau+x=x@x_x/A:%d/B:x@x-x&x-x/C:0", sequence[index - 1], sequence[index],
			sBound[sIndex] - sBound[sIndex - 1]);
	fullstr += tempstr;
	tempstr.format("pau\n");  // ky add
	monostr += tempstr; // ky add
	anchor = sIndex;
	while(wBound[wIndex - 1] < sBound[anchor])	// d2 ~ f2
		anchor--;
	tempstr.format("/D:%d/E:x@x+x/F:0", sIndex - anchor);
	fullstr += tempstr;
	anchor = sIndex;	// g1 ~ j3
	while(pBound[pIndex - 1] < sBound[anchor])
		anchor--;
	tempstr.format("/G:%d", sIndex - anchor);
	fullstr += tempstr;
	anchor = wIndex;
	while(pBound[pIndex - 1] < wBound[anchor])
		anchor--;
	tempstr.format("_%d/H:x=x@%d=1/I:0=0/J:%d+%d-%d\n", wIndex - anchor, pCount, sCount, wCount, pCount);
	fullstr += tempstr;
	csFile << fullstr;	//fullstr即為輸出的Label內容
	//csFile.close();
	if(pcsFile2 != NULL)
	{	// ky add: write out mono labels if needed
		(*pcsFile2) << monostr;
		//*pcsFile2.close();
	}
}

