/*
 * CTextProcess.cpp
 *
 *  Created on: 2018年9月28日
 *      Author: Jugo
 */

#include <memory.h>
#include <climits>
#include "CTextProcess.h"
#include "common.h"
#include "CStringArray.h"
#include "Word.h"
#include "CART.h"
#include "Phone.h"
#include "HTS_engine.h"

using namespace std;

#define FEATURE_DIM			14
#define CLUSTER				2
#define CART_MODEL			"model/CART_Model.bin"
#define CART_MODEL2			"model/CART_Model2.bin"

// ==================  注意順序不要改變!!!!  ==========================

static const char* Tonal[] = { "ㄓ", "ㄔ", "ㄕ", "ㄖ", "ㄗ", "ㄘ", "ㄙ",	// 7*2 + 1 = 15 (tonal-initial)
		"ㄧ", "ㄨ", "ㄩ", "ㄚ", "ㄛ", "ㄜ", "ㄞ", "ㄟ", "ㄠ", "ㄡ", "ㄢ", "ㄣ", "ㄤ", "ㄥ", "ㄝ", "ㄦ" }; // 16*2 + 6 = 38	(tonal-final)

static const char* ExtendedInitial[] = { "ㄅ", "ㄆ", "ㄇ", "ㄈ", "ㄉ", "ㄊ", "ㄋ", "ㄌ", "ㄍ", "ㄎ", "ㄏ", "ㄐ", "ㄑ", "ㄒ", "ㄓ", "ㄔ",
		"ㄕ", "ㄖ", "ㄗ", "ㄘ", "ㄙ", "ㄧ", "ㄨ", "ㄩ", "ㄅㄧ", "ㄆㄧ", "ㄇㄧ", "ㄉㄧ", "ㄊㄧ", "ㄋㄧ", "ㄌㄧ", "ㄐㄧ", "ㄑㄧ", "ㄒㄧ", "ㄓㄨ", "ㄔㄨ",
		"ㄕㄨ", "ㄖㄨ", "ㄗㄨ", "ㄘㄨ", "ㄙㄨ", "ㄍㄨ", "ㄎㄨ", "ㄏㄨ", "ㄉㄨ", "ㄊㄨ", "ㄋㄨ", "ㄌㄨ", "ㄐㄩ", "ㄑㄩ", "ㄒㄩ", "ㄋㄩ", "ㄌㄩ" };	// 53

//// 所以總共會有15 + 38 + 53 + 1(pau) = 107個sub-syllable model

static const char* Ph97Phoneme[] = { "jr", "chr", "shr", "r", "tz", "tsz", "sz",	// 0~6	(for tonal-initial)
		"yi", "wu", "yu", "a", "o", "e", "ai", "ei", "au", "ou", "an", "en", "ang", "ng", "eh", "er",// 7~22	(for toanl-final)
		"b", "p", "m", "f", "d", "t", "n", "l", "g", "k",					// 23~32	(for extended-initial)
		"h", "j", "ch", "sh", "jr", "chr", "shr", "r", "tz", "tsz",			// 33~42
		"sz", "yi", "wu", "yu", "bi", "pi", "mi", "di", "ti", "ni",			// 43~52
		"li", "ji", "chi", "shi", "ju", "chu", "shu", "ru", "tzu", "tsu",	// 53~62
		"su", "gu", "ku", "hu", "du", "tu", "nu", "lu", "jiu", "chiu",		// 63~72
		"shiu", "niu", "liu"											// 73~75
		};

// =====================================================================

const char *POStags[34] =
		{ " ", "VA", "VC", "VE", "VV", "NR", "NT", "NN", "LC", "PN", "DT", "CD", "OD", "M", "AD", "P", "CC", "CS",
				"DEC", "DEG", "DER", "DEV", "SP", "AS", "ETC", "MSP", "IJ", "ON", "PU", "JJ", "FW", "LB", "SB", "BA" };
struct thread_child_info
{
	BOOL* tPlayEnd;
} THREAD_CHILD_INFO;

CTextProcess::CTextProcess() :
		CartModel(new CART()), word(0), gduration_s(0), gduration_e(0), giSftIdx(0)
{
	loadModel();
}

CTextProcess::~CTextProcess()
{
	releaseModel();
}

void CTextProcess::loadModel()
{
	CartModel->LoadCARTModel();
}

void CTextProcess::releaseModel()
{
	if(CartModel)
		delete CartModel;
}

void CTextProcess::processTheText(const char *szText)
{
	int nIndex;
	int nLen = 0;
	int nCount = 0;
	CString strInput;
	CString strPart;

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
return;
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
		vector<int> tmpIdx(PWCluster.size(), lcount);
		indexArray.insert(indexArray.end(), tmpIdx.begin(), tmpIdx.end());
		AllBig5 += strBig5;
		AllPWCluster.insert(AllPWCluster.end(), PWCluster.begin(), PWCluster.end());
		AllPPCluster.insert(AllPPCluster.end(), PPCluster.begin(), PPCluster.end());

		k = l = 0;
		for(i = 0; i < word->wnum; i++)
		{
			for(j = 0; j < word->w_info[i].wlen; j++)
			{
				SID2Phone((word->w_info[i].phone[j]), &s[0]);
				SyllableTone[++sIndex] = (word->w_info[i].phone[j] % 10);
				CString delimiter = " ";
				CString pph = Phone2Ph97(s, SyllableTone[sIndex]);
				SplitString(pph, delimiter, PhoneSeq);
				SyllableBound[sIndex] = PhoneSeq.getSize() - 1;
				if(PWCluster[k] == 1)
				{
					WordBound[++wIndex] = SyllableBound[sIndex];
					if(PPCluster[l] == 2)
						PhraseBound[++pIndex] = WordBound[wIndex];
					++l;
				}
				++k;
			}
		}

		// generate label for current sentence
		ofstream csLabFile;
		csLabFile.open("gen/dlg.lab", ios::trunc);
		GenerateLabelFile(PhoneSeq, SyllableBound, WordBound, PhraseBound, sIndex, wIndex, pIndex, csLabFile, NULL);
		csLabFile.close();

		// 合成
		Synthesize("gen/dlg.lab", lcount);
	}
}

void CTextProcess::Synthesize(CString name, int c)
{
	/* hts_engine API */
	HTS_Engine engine;

	/* HTS voices */
	size_t num_voices;
	char **fn_voices;

	/* input label file name */
	char *labfn = NULL;

	/* output file pointers */
	FILE *durfp = NULL, *mgcfp = NULL, *lf0fp = NULL, *lpffp = NULL, *wavfp = NULL, *rawfp = NULL, *tracefp = NULL;

	/* interpolation weights */
	size_t num_interpolation_weights;

	/* initialize hts_engine API */
	HTS_Engine_initialize(&engine);

}

void CTextProcess::CartPrediction(CString &sentence, CString &strBig5, vector<int>& allPWCluster,
		vector<int>& allPPCluster)
{
	vector<int> wordpar;			// 當前的詞長
	vector<int> syllpos;			// 當前的字在詞中位置
	vector<int> cluster, cluster2;	// 韻律詞結尾=1, otherwise 0
	vector<int> pos;				// first: 幾字詞, second: POS tagging
	vector<int> pwBoundary;

	CString tempPOS = "";
	CStringArray tempPOSArray;
	CString cstemp = "";
	CString FileTitle = "";
	CString FileName = "";

	int i, j, k;
	int textNdx = 0;
	int valFeatureLW;
	int valFeaturePOS;

	/**************************************************/
	/*    將資料全行文字轉換成半形 針對數字部份             */
	/**************************************************/

	word = new CWord();
	word->GetSentence((unsigned char*) sentence.getBuffer(), &textNdx);
	word->GetWord();
	for(i = 0; i < word->wnum; ++i)
	{
		switch(word->w_info[i].wlen)
		{
		case 1:	//1字詞
			wordpar.push_back(1);
			syllpos.push_back(1);
			break;
		case 2: //2字詞
			wordpar.push_back(2);
			syllpos.push_back(2);
			wordpar.push_back(2);
			syllpos.push_back(3);
			break;
		case 3: //3字詞
			wordpar.push_back(3);
			syllpos.push_back(2);
			wordpar.push_back(3);
			syllpos.push_back(4);
			wordpar.push_back(3);
			syllpos.push_back(3);
			break;
		case 4:	//4字詞
			wordpar.push_back(4);
			syllpos.push_back(2);
			wordpar.push_back(4);
			syllpos.push_back(42);	//中前
			wordpar.push_back(4);
			syllpos.push_back(43);	//中後
			wordpar.push_back(4);
			syllpos.push_back(3);
			break;
		}
		strBig5 = strBig5 + word->w_info[i].big5;
		cstemp = cstemp + word->w_info[i].big5;
		cstemp = cstemp + " ";
	}

	/**
	 *  將文字分詞為 : 我們 來 做 垃圾
	 *  透過Standfor將詞性加入，變成: 我們/X 來/X 做/X 垃圾/X
	 *  將字詞屬性轉換成屬性標記
	 */
	tempPOS = "多型態角色語音智慧平台/X 我說一個故事給你們聽/X 要注意聽/X 千萬要注意聽/X 因為/X 如果沒聽到/X 你一定會問/X 你在說什麼/X"; // this is test............
	CString strDelim = " ";
	if(SplitString(tempPOS, strDelim, tempPOSArray) == 0)
		tempPOSArray.add(tempPOS);

	for(i = 0; i < tempPOSArray.getSize(); ++i)
	{
		int index = tempPOSArray[i].find("/", 0);
		cstemp.format("%s", tempPOSArray[i].mid(index + 1, tempPOSArray[i].getLength()));
		index /= 2;
		bool bAdded = false;
		for(j = 1; j < 34; ++j)
		{
			if(cstemp == POStags[j])
			{
				for(k = 0; k < index; k++)
					pos.push_back(j);
				bAdded = true;
				break;
			}
		}
		if(bAdded == false)
			for(k = 0; k < index; k++)
				pos.push_back(j);
	}

	/***********************************************************************
	 * syllable attribute資料結構產出，處理音韻詞的位置與音韻詞的字數狀態
	 ************************************************************************/
	// CART_Model
	for(i = 0; i < (int) syllpos.size(); ++i)						// 每一次iteration處理一個syllable的attribute
	{
		CART_DATA *pcdData = new CART_DATA();
		pcdData->clu = -1;

		for(j = 2; j >= -2; --j)
		{
			valFeatureLW = 0;
			if(((i - j) >= 0) && ((i - j) < (int) syllpos.size()))
			{
				valFeatureLW = wordpar[i - j];					// (LL/L/C/R/RR) syllable所在LW長度
			}
			pcdData->Att_Catagory.push_back(valFeatureLW);
		}
		pcdData->Att_Catagory.push_back(syllpos.size());			// Sentence length
		pcdData->Att_Catagory.push_back(i);							// Position in sentence (Forward)
		pcdData->Att_Catagory.push_back(syllpos.size() - i);		// Position in sentence (Backward)
		pcdData->Att_Catagory.push_back(syllpos[i]);				// Position in lexicon word
		for(j = 2; j >= -2; --j)
		{
			valFeaturePOS = 0;
			if(((i - j) >= 0) && ((i - j) < (int) syllpos.size()))
			{
				valFeaturePOS = pos[i - j];						// (LL/L/C/R/RR) syllable所在POS tags
			}
			pcdData->Att_Catagory.push_back(valFeaturePOS);
		}
		CartModel->TEST(pcdData);
		cluster.push_back(pcdData->clu);
		delete pcdData;
	}

	// 取得PWCluster數據
	for(i = 0; i < (int) cluster.size(); ++i)
	{
		allPWCluster.push_back(cluster[i]);
		if(cluster[i] == 1)
			pwBoundary.push_back(i);
	}
	cluster.clear();

	//============== CART_Model2 =================//
	for(i = 0, k = 0; i < (int) pwBoundary.size(); ++i, ++k)					// 每一次iteration處理一個syllable的attribute
	{
		CART_DATA *pcdData = new CART_DATA();
		pcdData->clu = 1;
		for(j = 2; j >= -2; j--)											// (LL/L/C/R/RR) syllable所在LW長度
		{
			valFeatureLW = 0;
			if(((k - j - 1) >= 0) && ((k - j) < (int) pwBoundary.size()))
				valFeatureLW = abs(pwBoundary[k - j] - pwBoundary[k - j - 1]);
			pcdData->Att_Catagory.push_back(valFeatureLW);
		}
		pcdData->Att_Catagory.push_back(pwBoundary.size());			// Sentence length
		pcdData->Att_Catagory.push_back(k);							// Position in sentence (Forward)
		pcdData->Att_Catagory.push_back(pwBoundary.size() - k);		// Position in sentence (Backward)
		pcdData->Att_Catagory.push_back(syllpos[i]);				// Position in lexicon word

		for(j = 2; j >= -2; j--)									// (LL/L/C/R/RR) syllable所在POS tags
		{
			valFeaturePOS = 0;
			if(((k - j) >= 0) && ((k - j) < (int) pwBoundary.size()))
				valFeaturePOS = pos[pwBoundary[k - j]];
			pcdData->Att_Catagory.push_back(valFeaturePOS);
		}
		CartModel->TEST2(pcdData);
		cluster.push_back(pcdData->clu);
		delete pcdData;
	}

	for(i = 0; i < (int) cluster.size(); ++i)
	{
		allPPCluster.push_back(cluster[i]);
	}

}

int CTextProcess::SplitString(CString& input, CString& delimiter, CStringArray& results)
{
	int iPos = 0;
	int newPos = -1;
	int sizeS2 = delimiter.getLength();
	int isize = input.getLength();

	vector<int> positions;
	newPos = input.find(delimiter, 0);
	if(newPos < 0)
	{
		return 0;
	}
	int numFound = 0;

	while(newPos > iPos)
	{
		++numFound;
		positions.push_back(newPos);
		iPos = newPos;
		newPos = input.find(delimiter, iPos + sizeS2 + 1);
	}

	for(int i = 0; i <= (int) positions.size(); ++i)
	{
		CString s;
		if(i == 0)
			s = input.mid(i, positions[i]);
		else
		{
			int offset = positions[i - 1] + sizeS2;
			if(offset < isize)
			{
				if(i == (int) positions.size())
					s = input.mid(offset);
				else if(i > 0)
					s = input.mid(positions[i - 1] + sizeS2, positions[i] - positions[i - 1] - sizeS2);
			}
		}
		if(s.getLength() > 0)
			results.add(s);
	}
	return numFound;
}

// Ph97: Extended initial + final
// 三個部分: part1: Extended initial
//			 part2 and part3: 含tone的tonal final or 單獨存在的 tonal initial
CString CTextProcess::Phone2Ph97(CString phone, int tone)
{
	CString result, tmp, whatever;
	result = tmp = "";
	char *buffer;
	int i, j, find;
	find = 0;
	j = 0;
	buffer = phone.getBuffer(0);
	int len = phone.getLength();  //一個字元2 bytes !!
	if((len == 2) || ((len == 4) && (tone != 1)))
	{	// 單獨母音或單獨子音
		for(i = 0; i < 23; i++)
		{
			if(memcmp(&buffer[j], Tonal[i], 2) == 0)
			{
				tmp.format("%s", Ph97Phoneme[i]);
				result += tmp;
//				tmp.Format(" %s",Ph97Phoneme[i]);
//				result += tmp;
				break;
			}
		}
		i++;
	}
	else if(((len == 4) && (tone == 1)) || ((len == 6) && (tone != 1)))
	{	// initial + final
		for(i = 0; (i < 53) && (find != 1); i++)
		{
			if(memcmp(&buffer[j], ExtendedInitial[i], 2) == 0)
			{
				tmp.format("%s ", Ph97Phoneme[i + 23]);
				result += tmp;		// part 1
				j += 2;
				for(i = 0; i < 23; i++)
				{
					if(memcmp(&buffer[j], Tonal[i], 2) == 0)
					{
						result += Ph97Phoneme[i];
						find = 1;
						break;
					}
				}
			}
		}
	}
	else
	{
		for(i = 24; (i < 53) && (find != 1); i++)
		{
			if(memcmp(&buffer[j], ExtendedInitial[i], 4) == 0)
			{
				tmp.format("%s ", Ph97Phoneme[i + 23]);
				result += tmp;
				j += 4;
				for(i = 0; i < 23; i++)
				{
					if((memcmp(&buffer[j], Tonal[i], 2)) == 0)
					{
						result += Ph97Phoneme[i];
						find = 1;
						break;
					}
				}
			}
		}
	}
	i--;
	if(tone != 5)
	{
		if((tone == 1) || (tone == 4))		// part 2
			result += "H ";
		else
			result += "L ";
		result += Ph97Phoneme[i];
		if((tone == 1) || (tone == 2))		// part 3
			result += "H";
		else
			result += "L";
	}
	else
	{
		tmp.format("M %sM", Ph97Phoneme[i]);
		result += tmp;
	}
	return result;
}

void CTextProcess::GenerateLabelFile(CStringArray& sequence, const int sBound[], const int wBound[], const int pBound[],
		const int sCount, const int wCount, const int pCount, ofstream& csFile, ofstream *pcsFile2)
{
	CString fullstr, tempstr; // fullstr: store all lines for full labels
	CString monostr; // store all lines for mono labels
	int sIndex, wIndex, pIndex; // syllable/word/phrase index for sBound/wBound/pBound
	sIndex = wIndex = pIndex = 0;
	fullstr = "";
	monostr = "";

	// output time information from cue. use timeinfo() to enable "outpu time information"
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

//void CTextProcess::GenerateBoundary(SYLLABLE_ATT& syllableAtt, std::vector<int>& vecCluster, int Model)
//{
//	int i, j;
//
//	for(i = 0; i < syllableAtt.size; ++i)
//	{
//		CART_DATA *pcdData = new CART_DATA();
//		pcdData->clu = syllableAtt.syllable_item[i].nCID;
//		for(j = 0; j < 5; ++j)
//		{
//			pcdData->Att_Catagory.push_back((const unsigned int) syllableAtt.syllable_item[i].valFeatureLW[j]);
//		}
//		pcdData->Att_Catagory.push_back((const unsigned int) syllableAtt.syllable_item[i].Sen_Length);
//		pcdData->Att_Catagory.push_back((const unsigned int) syllableAtt.syllable_item[i].F_PositionInSen);
//		pcdData->Att_Catagory.push_back((const unsigned int) syllableAtt.syllable_item[i].B_PositionInSen);
//		pcdData->Att_Catagory.push_back((const unsigned int) syllableAtt.syllable_item[i].PositionInWord);
//		for(j = 0; j < 5; ++j)
//		{
//			pcdData->Att_Catagory.push_back((const unsigned int) syllableAtt.syllable_item[i].valFeaturePOS[j]);
//		}
//		CartModel->TEST(pcdData);
//		vecCluster.push_back(pcdData->clu);
//		delete pcdData; // ky add: release memory
//	}
//}

