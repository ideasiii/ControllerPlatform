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
#include "CString.h"
#include "HTS_engine.h"
#include "hts_engine.h"
#include "CConvert.h"
#include "utility.h"
#include "WordInfo.h"
#include "utf8.h"
#include <map>

//*****for domain socket client*****//
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//char *socket_path = "/home/johann/ipc_tmp/ipc_tmp.txt";  //original test
char *socket_path = "/data/opt/ipc_tmp/ipc_tmp.txt";
//***************************//

using namespace std;

#define FEATURE_DIM				14
#define CLUSTER						2
#define CART_MODEL				"model/CART_Model.bin"
#define CART_MODEL2			"model/CART_Model2.bin"
//#define HMM_MODEL				"model/hmm.htsvoice"
//#define HMM_MODEL				"model/hmm_original.htsvoice"
#define HMM_MODEL				"model/hmm_adapt.htsvoice"
#define WORD_MODEL			"model/"
#define PATH_WAVE					"/data/opt/tomcat/webapps/tts/"

#define max_size 1000

// ==================  注意順序不要改變!!!!  ==========================

static const char* Tonal[] = { "ㄓ", "ㄔ", "ㄕ", "ㄖ", "ㄗ", "ㄘ", "ㄙ",	// 7*2 + 1 = 15 (tonal-initial)
		"ㄧ", "ㄨ", "ㄩ", "ㄚ", "ㄛ", "ㄜ", "ㄞ", "ㄟ", "ㄠ", "ㄡ", "ㄢ", "ㄣ", "ㄤ", "ㄥ", "ㄝ", "ㄦ" }; // 16*2 + 6 = 38	(tonal-final)

static const char* ExtendedInitial[] = { "ㄅ", "ㄆ", "ㄇ", "ㄈ", "ㄉ", "ㄊ", "ㄋ", "ㄌ", "ㄍ", "ㄎ", "ㄏ", "ㄐ", "ㄑ", "ㄒ", "ㄓ", "ㄔ",
		"ㄕ", "ㄖ", "ㄗ", "ㄘ", "ㄙ", "ㄧ", "ㄨ", "ㄩ", "ㄅㄧ", "ㄆㄧ", "ㄇㄧ", "ㄉㄧ", "ㄊㄧ", "ㄋㄧ", "ㄌㄧ", "ㄐㄧ", "ㄑㄧ", "ㄒㄧ", "ㄓㄨ", "ㄔㄨ",
		"ㄕㄨ", "ㄖㄨ", "ㄗㄨ", "ㄘㄨ", "ㄙㄨ", "ㄍㄨ", "ㄎㄨ", "ㄏㄨ", "ㄉㄨ", "ㄊㄨ", "ㄋㄨ", "ㄌㄨ", "ㄐㄩ", "ㄑㄩ", "ㄒㄩ", "ㄋㄩ", "ㄌㄩ" };	// 53

//// 所以總共會有15 + 38 + 53 + 1(pau) = 107個sub-syllable model

static const char* Ph97Phoneme[] = { "jr", "chr", "shr", "r", "tz", "tsz", "sz",	// 0~6	(for tonal-initial)
		"yi", "wu", "yu", "a", "o", "e", "ai", "ei", "au", "ou", "an", "en", "ang", "ng", "eh", "er",// 7~22	(for toanl-final)
		"b", "p", "m", "f", "d", "t", "n", "l", "g", "k",	// 23~32	(for extended-initial)
		"h", "j", "ch", "sh", "jr", "chr", "shr", "r", "tz", "tsz",		// 33~42
		"sz", "yi", "wu", "yu", "bi", "pi", "mi", "di", "ti", "ni",		// 43~52
		"li", "ji", "chi", "shi", "ju", "chu", "shu", "ru", "tzu", "tsu",		// 53~62
		"su", "gu", "ku", "hu", "du", "tu", "nu", "lu", "jiu", "chiu",	// 63~72
		"shiu", "niu", "liu"											// 73~75
		};

// =====================================================================
// standford
const char *POStags[34] =
		{ " ", "VA", "VC", "VE", "VV", "NR", "NT", "NN", "LC", "PN", "DT", "CD", "OD", "M", "AD", "P", "CC", "CS",
				"DEC", "DEG", "DER", "DEV", "SP", "AS", "ETC", "MSP", "IJ", "ON", "PU", "JJ", "FW", "LB", "SB", "BA" };

CTextProcess::CTextProcess() :
		CartModel(new CART()), convert(new CConvert), word(new CWord)
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
	word->InitWord(WORD_MODEL);
}

void CTextProcess::releaseModel()
{
	if (CartModel)
		delete CartModel;
	if (convert)
		delete convert;
	if (word)
		delete word;
}

int CTextProcess::processTheText(const char *szText, CString &strWavePath)
{

	time_t rawtime;
	time(&rawtime);

	CString strLabelName;

	int nSypollCount;
	int nIndex;
	int nLen = 0;
	int nCount = 0;
	CString strInput;
	CString strPart;
	char s[10];
	int SyllableBound[100];		// syllable邊界
	int WordBound[100];			// word boundary, tts 斷詞結果
	int PhraseBound[20];		// phrase boundary
	int *gduration_s = NULL;
	int *gduration_e = NULL;
	int giSftIdx = 0;
	int playcount = 0;
	int i, j, k, l, sIndex, wIndex, pIndex;
	CStringArray SentenceArray;
	CString AllBig5;
	int textNdx;
	WORD_PACKAGE wordPackage;

	strWavePath.format("%s%ld.wav", PATH_WAVE, rawtime);
	strLabelName.format("label/%ld.lab", rawtime);
	AllBig5 = "";

	_log("[CTextProcess] processTheText Input Text: %s", szText);
	strInput = szText;
	strInput.trim();

	WordExchange(strInput);
	_log("[CTextProcess] processTheText SpanExcluding and Word Exchange Text: %s", strInput.getBuffer());

	string strFinded;
	while ((i = strInput.findOneOf(vWordWrap, strFinded)) != -1)
	{
		for (vector<string>::iterator vdel_it = vWordDel.begin(); vWordDel.end() != vdel_it; ++vdel_it)
		{
			strInput.left(i).replace(vdel_it->c_str(), "");
		}
		SentenceArray.add(strInput.left(i));
		strInput = strInput.right(strInput.getLength() - i - strFinded.length());
	}

	if (0 >= SentenceArray.getSize() || 0 < strInput.getLength())
	{
		SentenceArray.add(strInput);
	}

	for (i = 0; i < SentenceArray.getSize(); ++i)
	{
		_log("[CTextProcess] processTheText Sentence: %s", SentenceArray[i].getBuffer());
	}

	for (int lcount = 0; lcount < (int) SentenceArray.getSize(); ++lcount)
	{
		CStringArray PhoneSeq;	// 紀錄整個utterance的phone model sequence  音節  音素
		CString strBig5;
		vector<int> PWCluster;
		vector<int> PPCluster;

		_log("initial boundaries and indexes for a sentence");
		sIndex = wIndex = pIndex = 0;
		SyllableBound[0] = WordBound[0] = PhraseBound[0] = -1;
		for (i = 1; i < 100; ++i)
			SyllableBound[i] = WordBound[i] = 0;
		for (i = 1; i < 20; ++i)
			PhraseBound[i] = 0;

		wordPackage.clear();
		wordPackage.strText = SentenceArray[lcount].getBuffer();
		wordPackage.txt_len = utf8len(wordPackage.strText.c_str());

		word->GetWord(wordPackage);

		if (-1 == CartPrediction(SentenceArray[lcount], strBig5, PWCluster, PPCluster, wordPackage))
			continue;
		AllBig5 += strBig5;

		k = l = 0;
		for (i = 0; i < wordPackage.wnum; ++i) // 字詞數
		{
			for (j = 0; j < wordPackage.vecWordInfo[i].wlen; ++j)   // 詞的字數
			{
				_log("[CTextProcess] processTheText Text: %s Phone: %s", wordPackage.vecWordInfo[i].strSentence.c_str(),
						wordPackage.vecWordInfo[i].strPhone[j].c_str());

				vector<string> vData;
				spliteData(const_cast<char *>(wordPackage.vecWordInfo[i].strPhone[j].c_str()), " ", vData);

				for (vector<string>::iterator it = vData.begin(); vData.end() != it; ++it)  // 因素分解
				{
					PhoneSeq.add(*it);
				}
				++sIndex;

				SyllableBound[sIndex] = PhoneSeq.getSize() - 1;
				//_log("======================> SyllableBound[%d] = %d", sIndex, SyllableBound[sIndex]);

				if (!PWCluster.empty() && PWCluster[k] == 1)
				{
					WordBound[++wIndex] = SyllableBound[sIndex];
					if (PPCluster[l] == 2)
						PhraseBound[++pIndex] = WordBound[wIndex];
					++l;
				}
				++k;
			}
		}
		_log("=============== 合成音標檔 %s===============", strLabelName.getBuffer());
		ofstream csLabFile;
		csLabFile.open(strLabelName.getBuffer(), ios::app);
		GenerateLabelFile(PhoneSeq, SyllableBound, WordBound, PhraseBound, sIndex, wIndex, pIndex, csLabFile, NULL,
				gduration_s, gduration_e, giSftIdx);
		csLabFile.close();
		PhoneSeq.removeAll();
	}
	_log("[CTextProcess] processTheText AllBig5: %s", AllBig5.getBuffer());

	_log("=============== 合成聲音檔 %s===============", strWavePath.getBuffer());
	return Synthesize(HMM_MODEL, strWavePath.getBuffer(), strLabelName.getBuffer());

}

int CTextProcess::Synthesize(const char* szModelName, const char* szWaveName, const char* szLabel)
{

	int nResult;
	char** param = new char*[12];
	param[0] = const_cast<char*>("hts_engine");
	param[1] = const_cast<char*>("-m");
	param[2] = const_cast<char*>(szModelName);
	param[3] = const_cast<char*>("-ow");
	param[4] = const_cast<char*>(szWaveName);
	param[5] = const_cast<char*>(szLabel);
	param[6] = const_cast<char*>("-fm");
	param[7] = const_cast<char*>("1");
	param[8] = const_cast<char*>("-b");
	param[9] = const_cast<char*>("0.0");
	param[10] = const_cast<char*>("-r");
	param[11] = const_cast<char*>("1.0");
	_log("[CTextProcess] Synthesize Model Name: %s Wave Name: %s Label Name: %s", szModelName, szWaveName, szLabel);
	nResult = htsSynthesize(12, param);
	delete param;
	return nResult;

	//=============windows version========================//
	int i;
	CString command;
	CStringArray strCommandArray;
	command.format("%s", szLabel);	// argument 1
	strCommandArray.add(command);
	command.replace(".lab", ".f0");			// argument 2
	strCommandArray.add(command);
	command.replace(".f0", ".raw");			// argument 3
	strCommandArray.add(command);
	command.replace(".raw", ".trace");			// argument 4
	strCommandArray.add(command);

	char **commandLine = new char*[strCommandArray.getSize()];
	for (i = 0; i < strCommandArray.getSize(); i++)
	{
		commandLine[i] = new char[strCommandArray[i].getLength() + 2];		// CString GetLength回傳的大小不包含"\0" (預留空間給\0)
		strcpy(commandLine[i], strCommandArray[i].getBuffer(strCommandArray[i].getLength()));		// strcpy會在最後加上"\0"
		strCommandArray[i].releaseBuffer();
	}

	_log("[CTextProcess] Synthesize to Windows version: %s %s %s %s", commandLine[0], commandLine[1], commandLine[2],
			commandLine[3]);
	hts_engine(commandLine, 1.2f, -0.03f, 0);

}

int CTextProcess::CartPrediction(CString &sentence, CString &strBig5, vector<int>& allPWCluster,
		vector<int>& allPPCluster, WORD_PACKAGE &wordPackage)
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

	unsigned long ulIndex;
	unsigned long ulLength;
	bool bAdded = false;
	int i, j, k;
	int textNdx = 0;
	int valFeatureLW;
	int valFeaturePOS;
	_log("[CTextProcess] CartPrediction sentence: %s  wnum: %d", sentence.getBuffer(), wordPackage.wnum);

//	const char* test[] = {"/AD ", "/VV ", "/NN "};  //硬給詞性

    //***** domain socket connect *****//
	struct sockaddr_un addr;
	int fd;
	char receiveMessage[max_size];

	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(-1);
	}
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (*socket_path == '\0') {
		*addr.sun_path = '\0';
	  strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
	} else {
	  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
	}
	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
	  perror("connect error");
	  exit(-1);
	}
	//***************//

	for (i = 0; i < wordPackage.wnum; ++i)
	{
		switch (wordPackage.vecWordInfo[i].wlen)
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

		strBig5 = strBig5 + wordPackage.vecWordInfo[i].strSentence;
		cstemp = cstemp + wordPackage.vecWordInfo[i].strSentence ;
		cstemp = cstemp + " ";
	}

	//***** domain socket *****//

	memset(receiveMessage, 0, sizeof(receiveMessage));
	write(fd, cstemp, cstemp.getLength());
	printf("Input: %s\n", cstemp.getBuffer());
	read(fd,receiveMessage,sizeof(receiveMessage));
	printf("%s\n", receiveMessage);
	CString cstring = receiveMessage;
	cstemp = cstring;

	//********************//

	_log("================ len %d=================", cstemp.getLength());
	if (0 >= cstemp.getLength())
	{
		_log("[CTextProcess] CartPrediction Dictionary loss : %s", sentence.getBuffer());
		return -1;
	}

	_log("[CTextProcess] CartPrediction String  : %s , cstemp : %s", strBig5.getBuffer(), cstemp.getBuffer());

	/**
	 *  將文字分詞為 : 我們 來 做 垃圾
	 *  透過Standfor將詞性加入，變成: 我們/X 來/X 做/X 垃圾/X
	 *  將字詞屬性轉換成屬性標記
	 */

	vector<string> vData;
	spliteData(const_cast<char *>(cstemp.getBuffer()), " ", vData);
	for (vector<string>::iterator it = vData.begin(); vData.end() != it; ++it)
	{
		//	_log("[CTextProcess] CartPrediction spliteData: %s", it->c_str());
		ulIndex = it->find("/", 0);
		ulLength = it->length();
		tempPOS.format("%s", it->substr(ulIndex + 1, ulLength).c_str());
		ulIndex /= 3;
		bAdded = false;
		for (j = 1; 34 > j; ++j)
		{
			if (!tempPOS.Compare(POStags[j]))
			{

					pos.push_back(j);
					for (k = 1; k < (int) ulIndex; ++k)
								{
									pos.push_back(j);
								}

					bAdded = true;
					break;

			}
			if (bAdded)
				break;
		}

	}


//	_log("syllable attribute資料結構產出，處理音韻詞的位置與音韻詞的字數狀態");
//	_log("============== CART_Model =================");
	_log("[CTextProcess] CartPrediction syllpos size: %d", (int) syllpos.size());
	for (i = 0; i < (int) syllpos.size(); ++i) // 每一次iteration處理一個syllable的attribute
	{
		CART_DATA *pcdData = new CART_DATA();
		pcdData->clu = -1;

		for (j = 2; j >= -2; --j)
		{
			valFeatureLW = 0;
			if (((i - j) >= 0) && ((i - j) < (int) syllpos.size()))
			{
				valFeatureLW = wordpar[i - j];	// (LL/L/C/R/RR) syllable所在LW長度
			}
			pcdData->Att_Catagory.push_back(valFeatureLW);

		}
		//_log("Sentence length: %d Forward: %d Backward: %d lexicon: %d", pwBoundary.size(), k, pwBoundary.size() - k,	syllpos[i]);
		pcdData->Att_Catagory.push_back(syllpos.size());	// Sentence length
		pcdData->Att_Catagory.push_back(i);	// Position in sentence (Forward)
		pcdData->Att_Catagory.push_back(syllpos.size() - i);	// Position in sentence (Backward)
		pcdData->Att_Catagory.push_back(syllpos[i]);	// Position in lexicon word



		for (j = 2; j >= -2; --j)
		{
			valFeaturePOS = 0;
			if (((i - j) >= 0) && ((i - j) < (int) syllpos.size()))
			{
				valFeaturePOS = pos[i - j];	// (LL/L/C/R/RR) syllable所在POS tags
			}
			pcdData->Att_Catagory.push_back(valFeaturePOS);

		}

		CartModel->TEST(pcdData);
		cluster.push_back(pcdData->clu);

		delete pcdData;
	}

//	_log("取得PWCluster數據");
	for (i = 0; i < (int) cluster.size(); ++i)
	{
		allPWCluster.push_back(cluster[i]);
		_log("PWCluster數據: %d", cluster[i]);
		if (cluster[i] == 1)
			pwBoundary.push_back(i);

	}

	cluster.clear();

//	_log("============== CART_Model2 =================");
	for (i = 0, k = 0; i < (int) pwBoundary.size(); ++i, ++k)	// 每一次iteration處理一個syllable的attribute
	{
		CART_DATA *pcdData = new CART_DATA();
		pcdData->clu = 1;
		for (j = 2; j >= -2; j--)				// (LL/L/C/R/RR) syllable所在LW長度
		{
			valFeatureLW = 0;
			if (((k - j - 1) >= 0) && ((k - j) < (int) pwBoundary.size()))
				valFeatureLW = abs(pwBoundary[k - j] - pwBoundary[k - j - 1]);
			pcdData->Att_Catagory.push_back(valFeatureLW);
		}
//		_log("Sentence length: %d Forward: %d Backward: %d lexicon: %d", pwBoundary.size(), k, pwBoundary.size() - k,	syllpos[i]);
		pcdData->Att_Catagory.push_back(pwBoundary.size());	// Sentence length
		pcdData->Att_Catagory.push_back(k);	// Position in sentence (Forward)
		pcdData->Att_Catagory.push_back(pwBoundary.size() - k);	// Position in sentence (Backward)
		pcdData->Att_Catagory.push_back(syllpos[i]);	// Position in lexicon word

		for (j = 2; j >= -2; j--)			// (LL/L/C/R/RR) syllable所在POS tags
		{
			valFeaturePOS = 0;
			if (((k - j) >= 0) && ((k - j) < (int) pwBoundary.size()))
				valFeaturePOS = pos[pwBoundary[k - j]];
			pcdData->Att_Catagory.push_back(valFeaturePOS);
					//_log("valFeaturePOS: %d", valFeaturePOS);
		}
//		_log("CART TEST2");
		CartModel->TEST2(pcdData);

		cluster.push_back(pcdData->clu);
		_log("cluster push: %d", pcdData->clu);
		delete pcdData;
	}

//	_log("取得PPCluster數據");
	for (i = 0; i < (int) cluster.size(); ++i)
	{
		allPPCluster.push_back(cluster[i]);
		_log("PPCluster數據: %d", cluster[i]);
	}
	cluster.clear();
	return 0;
}

// Ph97: Extended initial + final
// 三個部分: part1: Extended initial
//			 		 part2 and part3: 含tone的tonal final or 單獨存在的 tonal initial
CString CTextProcess::Phone2Ph97(char* phone, int tone)
{
	CString result, tmp, whatever;
	result = tmp = "";
	char *buffer;
	int i, j, find;
	find = 0;
	j = 0;
	buffer = phone;

	/************ UTF8 轉 Big5 **********************************/
	char *big5 = 0;
	char **pBig5 = &big5;
	convert->UTF8toBig5(phone, pBig5);
	int len = strlen(big5);  //一個字元2 bytes !!
	free(big5);

	if ((len == 2) || ((len == 4) && (tone != 1)))
	{
		for (i = 0; i < 23; i++)
		{
			if (memcmp(&buffer[j], Tonal[i], 3) == 0)
			{
				tmp.format("%s", Ph97Phoneme[i]);
				result += tmp;
				break;
			}
		}
		i++;
	}
	else if (((len == 4) && (tone == 1)) || ((len == 6) && (tone != 1)))
	{
		for (i = 0; (i < 53) && (find != 1); i++)
		{
			if (memcmp(&buffer[j], ExtendedInitial[i], 3) == 0)
			{
				tmp.format("%s ", Ph97Phoneme[i + 23]);
				result += tmp;		// part 1
				j += 3;
				for (i = 0; i < 23; i++)
				{
					if (memcmp(&buffer[j], Tonal[i], 3) == 0)
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
		for (i = 24; (i < 53) && (find != 1); i++)
		{
			if (memcmp(&buffer[j], ExtendedInitial[i], 6) == 0)
			{
				tmp.format("%s ", Ph97Phoneme[i + 23]);
				result += tmp;
				j += 6;
				for (i = 0; i < 23; i++)
				{
					if ((memcmp(&buffer[j], Tonal[i], 3)) == 0)
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
	if (tone != 5)
	{
		if ((tone == 1) || (tone == 4))		// part 2
			result += "H ";
		else
			result += "L ";
		result += Ph97Phoneme[i];
		if ((tone == 1) || (tone == 2))		// part 3
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

CString CTextProcess::GenerateLabelFile(CStringArray& sequence, const int sBound[], const int wBound[],
		const int pBound[], const int sCount, const int wCount, const int pCount, ofstream& csFile, ofstream *pcsFile2,
		int *gduration_s, int *gduration_e, int giSftIdx)
{
	CString fullstr, tempstr; // fullstr: store all lines for full labels
	CString monostr; // store all lines for mono labels
	int sIndex, wIndex, pIndex; // syllable/word/phrase index for sBound/wBound/pBound
	sIndex = wIndex = pIndex = 0;
	fullstr = "";
	monostr = "";

	// output time information from cue. use timeinfo() to enable "outpu time information"
//	char timebuf[25]; // tag of time. calculated from cue and syllabel boundaries
	if (gduration_s != NULL) // output time info for the first pause label
	{
		int tmp = 0;
		if (gduration_s[1] - 1000000 > 0)
			tmp = gduration_s[1] - 1000000;
		tempstr.format("%10d %10d ", tmp, gduration_s[1]);
		fullstr += tempstr;
		monostr += tempstr;
	}

	// p1^p2-p3+p4=p5@p6_p7/A:a3/B:b3@b4-b5&b6-b7/C:c3/D:d2/E:e2@e3+e4
	// /F:f2/G:g1_g2/H:h1=h2@h3=h4/I:i1=i2/J:j1+j2-j3
	tempstr.format("x^x-pau+%s=%s@x_x/A:0/B:x@x-x&x-x/C:%d/D:0/E:x@x+x", sequence[0].getBuffer(),
			sequence[1].getBuffer(), sBound[1] + 1);	// current phoneme = pause (pau);

	fullstr += tempstr; // ky modify: don't initial fullstr, since I'll do it myself.
	tempstr.format("pau\n"); // ky add: for mono
	monostr += tempstr; // ky add: for mono
	int anchor, anchor2;
	anchor = anchor2 = 0;

	while (sBound[anchor] != wBound[1]) 	// f2
	{
		anchor++;
		if (100 <= anchor)
			break;
	}

	tempstr.format("/F:%d/G:0_0/H:x=x@1=%d", anchor, pCount);
	fullstr += tempstr;

	if (pCount == 1)	// i1, i2
		tempstr.format("/I:%d=%d", sCount, wCount);
	else
	{
		anchor = 0;
		while (sBound[anchor] != pBound[1])	// i1
			anchor++;
		tempstr.format("/I:%d", anchor);
		fullstr += tempstr;
		anchor = 0;
		while (wBound[anchor] != pBound[1])	// i2
			anchor++;
		tempstr.format("=%d", anchor);
		fullstr += tempstr;
	}

	tempstr.format("/J:%d+%d-%d\n", sCount, wCount, pCount);
	fullstr += tempstr;
	int iMM = INT_MAX; //index mm: syllable_phone   phone index. since the mm is assigned from sylla_p[ll], the value is initialed as maximum value, and reassigned in the loop
	for (int index = 0; index < sequence.getSize(); index++) // index = current phone
	{
		if (sBound[sIndex] < index)
			sIndex++;
		if (wBound[wIndex] < index)
			wIndex++;
		if (pBound[pIndex] < index)
			pIndex++;

		// ky add: add time info for each line of label.
		if (gduration_s != NULL)
		{
			// simulate indexes in old version: window_synthesis_demo: textpross::labelgen()
			int iLL = sIndex - 1;	// index ll: word_syllable   syllable index
			int iSy_p_ll = sBound[iLL] + 1;			// sylla_p[ll]
			int iSy_p_ll_1 = sBound[iLL + 1] + 1;		// sylla_p[ll+1]
			if (iMM >= iSy_p_ll_1 + 2)	// index mm: syllable_phone   phone index. simulate the loop of index mm
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
			tempstr.format("%s\n", sequence[index].getBuffer());
			monostr += tempstr;
		}
		//

		// p1~p5
		if (index < 2)
		{
			if (index == 0)
			{
				if (sequence.getSize() == 1)
					tempstr.format("x^pau-%s+x=x", sequence[index].getBuffer());
				else if (sequence.getSize() == 2)
					tempstr.format("x^pau-%s+%s=x", sequence[index].getBuffer(), sequence[index + 1].getBuffer());
				else
					tempstr.format("x^pau-%s+%s=%s", sequence[index].getBuffer(), sequence[index + 1].getBuffer(),
							sequence[index + 2].getBuffer());
			}
			else	// index == 1
			{
				if (sequence.getSize() == 2)
					tempstr.format("pau^%s-%s+x=x", sequence[index - 1].getBuffer(), sequence[index].getBuffer());
				else if (sequence.getSize() == 3)
					tempstr.format("pau^%s-%s+%s=x", sequence[index - 1].getBuffer(), sequence[index].getBuffer(),
							sequence[index + 1].getBuffer());
				else
					tempstr.format("pau^%s-%s+%s=%s", sequence[index - 1].getBuffer(), sequence[index].getBuffer(),
							sequence[index + 1].getBuffer(), sequence[index + 2].getBuffer());
			}
		}
		else if (index > sequence.getSize() - 3)
		{
			if (index == sequence.getSize() - 2)
				tempstr.format("%s^%s-%s+%s=pau", sequence[index - 2].getBuffer(), sequence[index - 1].getBuffer(),
						sequence[index].getBuffer(), sequence[index + 1].getBuffer());
			else
				tempstr.format("%s^%s-%s+pau=x", sequence[index - 2].getBuffer(), sequence[index - 1].getBuffer(),
						sequence[index].getBuffer());
		}
		else
			tempstr.format("%s^%s-%s+%s=%s", sequence[index - 2].getBuffer(), sequence[index - 1].getBuffer(),
					sequence[index].getBuffer(), sequence[index + 1].getBuffer(), sequence[index + 2].getBuffer());
		fullstr += tempstr;

		// p6, p7
		tempstr.format("@%d_%d", index - sBound[sIndex - 1], sBound[sIndex] + 1 - index);
		fullstr += tempstr;

		// a3, b3
		if (sIndex == 1)
			tempstr.format("/A:0/B:%d", sBound[sIndex] - sBound[sIndex - 1]);
		else
			tempstr.format("/A:%d/B:%d", sBound[sIndex - 1] - sBound[sIndex - 2], sBound[sIndex] - sBound[sIndex - 1]);
		fullstr += tempstr;

		// b4, b5
		anchor = sIndex;
		while (wBound[wIndex - 1] < sBound[anchor])
			anchor--;
		tempstr.format("@%d", sIndex - anchor);
		fullstr += tempstr;
		anchor = sIndex;
		while (wBound[wIndex] > sBound[anchor])
			anchor++;
		tempstr.format("-%d", anchor - sIndex + 1);
		fullstr += tempstr;

		// b6, b7
		anchor = sIndex;
		while (pBound[pIndex - 1] < sBound[anchor])
			anchor--;
		tempstr.format("&%d", sIndex - anchor);
		fullstr += tempstr;
		anchor = sIndex;
		while (pBound[pIndex] > sBound[anchor])
			anchor++;
		tempstr.format("-%d", anchor - sIndex + 1);
		fullstr += tempstr;

		// c3
		if (sIndex == sCount)
			tempstr.format("/C:0");
		else
			tempstr.format("/C:%d", sBound[sIndex + 1] - sBound[sIndex]);
		fullstr += tempstr;

		// d2
		if (wIndex == 1)
			tempstr.format("/D:0");
		else
		{
			anchor = sIndex;
			while (sBound[anchor] != wBound[wIndex - 1])
				anchor--;
			anchor2 = anchor;
			while (wBound[wIndex - 2] < sBound[anchor2])
				anchor2--;
			tempstr.format("/D:%d", anchor - anchor2);
		}
		fullstr += tempstr;
		// e2
		anchor = sIndex;
		while (wBound[wIndex - 1] < sBound[anchor])
			anchor--;
		anchor2 = sIndex;
		while (wBound[wIndex] > sBound[anchor2])
			anchor2++;
		tempstr.format("/E:%d", anchor2 - anchor);
		fullstr += tempstr;

		// e3, e4
		anchor = wIndex;
		while (pBound[pIndex - 1] < wBound[anchor])
			anchor--;
		tempstr.format("@%d", wIndex - anchor);
		fullstr += tempstr;
		anchor = wIndex;
		while (pBound[pIndex] > wBound[anchor])
			anchor++;
		tempstr.format("+%d", anchor - wIndex + 1);
		fullstr += tempstr;

		// f2:  #of syllable in the next next word
		anchor = sIndex;
		while (sBound[anchor] < wBound[wIndex])
			anchor++;
		anchor2 = anchor;	// anchor2: where the next word start
		while (sBound[anchor] < wBound[wIndex + 1])
			anchor++;
		tempstr.format("/F:%d", anchor - anchor2);
		fullstr += tempstr;

		// g1:	#of syllables in the previous phrase
		// g2:	#of words in the previous phrase
		if (pIndex == 1)
			tempstr.format("/G:0_0");
		else
		{
			anchor = sIndex;
			while (sBound[anchor] > pBound[pIndex - 1])
				anchor--;
			anchor2 = anchor;
			while (pBound[pIndex - 2] < sBound[anchor2])
				anchor2--;
			tempstr.format("/G:%d", anchor - anchor2);
			fullstr += tempstr;
			anchor = wIndex;
			while (wBound[anchor] > pBound[pIndex - 1])
				anchor--;
			anchor2 = anchor;
			while (pBound[pIndex - 2] < wBound[anchor2])
				anchor2--;
			tempstr.format("_%d", anchor - anchor2);
		}
		fullstr += tempstr;

		// h1:	#of syllables in the current phrase
		// h2:	#of words in the current phrase
		anchor = anchor2 = sIndex;
		while (pBound[pIndex - 1] < sBound[anchor])
			anchor--;
		while (pBound[pIndex] > sBound[anchor2])
			anchor2++;
		tempstr.format("/H:%d", anchor2 - anchor);
		fullstr += tempstr;
		anchor = anchor2 = wIndex;
		while (pBound[pIndex - 1] < wBound[anchor])
			anchor--;
		while (pBound[pIndex] > wBound[anchor2])
			anchor2++;
		tempstr.format("=%d", anchor2 - anchor);
		fullstr += tempstr;

		tempstr.format("@%d=%d", pIndex, pCount - pIndex + 1);	// h3, h4
		fullstr += tempstr;
		// i1, i2
		if (pCount == 1)
			tempstr.format("/I:0=0");
		else
		{
			anchor = anchor2 = sIndex;
			while (pBound[pIndex] > sBound[anchor])
				anchor++;
			anchor2 = anchor;
			while (pBound[pIndex + 1] > sBound[anchor])
				anchor++;
			tempstr.format("/I:%d", anchor - anchor2);
			fullstr += tempstr;
			anchor = anchor2 = wIndex;
			while (pBound[pIndex] > wBound[anchor])
				anchor++;
			anchor2 = anchor;
			while (pBound[pIndex + 1] > wBound[anchor])
				anchor++;
			tempstr.format("=%d", anchor - anchor2);
			fullstr += tempstr;
		}
		tempstr.format("/J:%d+%d-%d\n", sCount, wCount, pCount);	// j1,j2,j3
		fullstr += tempstr;
	}
	//	index--;
	int index = sequence.getSize() - 1;	//20091211 rosy edit for .Net 取代上面那行 index --

	// ky add: add time info for pau at the end of the sentence
	if (gduration_s != NULL)
	{
		int tmp = 0;
		int iCount_2 = sCount;  // simulate index for count[2]
		giSftIdx += sCount; // update global shift of syllable index for each sentence
		tmp = gduration_e[iCount_2] + 1000000;
		tempstr.format("%10d %10d ", gduration_e[iCount_2], tmp);
		fullstr += tempstr;
		monostr += tempstr;
	}
	//

	tempstr.format("%s^%s-pau+x=x@x_x/A:%d/B:x@x-x&x-x/C:0", sequence[index - 1].getBuffer(),
			sequence[index].getBuffer(), sBound[sIndex] - sBound[sIndex - 1]);
	fullstr += tempstr;
	tempstr.format("pau\n");  // ky add
	monostr += tempstr; // ky add
	anchor = sIndex;
	while (wBound[wIndex - 1] < sBound[anchor])	// d2 ~ f2
		anchor--;
	tempstr.format("/D:%d/E:x@x+x/F:0", sIndex - anchor);
	fullstr += tempstr;
	anchor = sIndex;	// g1 ~ j3
	while (pBound[pIndex - 1] < sBound[anchor])
		anchor--;
	tempstr.format("/G:%d", sIndex - anchor);
	fullstr += tempstr;
	anchor = wIndex;
	while (pBound[pIndex - 1] < wBound[anchor])
		anchor--;
	tempstr.format("_%d/H:x=x@%d=1/I:0=0/J:%d+%d-%d\n", wIndex - anchor, pCount, sCount, wCount, pCount);
	fullstr += tempstr;
	csFile << fullstr;	//fullstr即為輸出的Label內容
	//csFile.close();
	if (pcsFile2 != NULL)
	{	// ky add: write out mono labels if needed
		(*pcsFile2) << monostr;
		//*pcsFile2.close();
	}

	return fullstr;
}

void CTextProcess::dumpWordData()
{
	char byte;								//詞的BYTE數 (1 byte)
	unsigned char attr[4];           //(4 bytes)
	unsigned char big5[20];  	    //big5,一字二占2 byte (20 bytes)
	unsigned int counter;          //字數*2 (1 byte)
	short phone[10];  //發音代碼 (10 bytes)
	CString strData;

	ofstream csWordFile1("WordData.txt", ios::app);

	ifstream cf("model/WORD.DAT", std::ifstream::binary);
	cf.seekg(0, cf.end);
	int fend = cf.tellg();
	cf.seekg(0, cf.beg);
	_log("word data size=%d", fend);

	CConvert convert;
	while (cf.tellg() != fend)
	{
		cf.read(reinterpret_cast<char *>(&byte), 1);
		cf.read(reinterpret_cast<char *>(attr), 4);
		cf.read(reinterpret_cast<char *>(big5), 20);
		cf.read(reinterpret_cast<char *>(&counter), sizeof(int));
		cf.read(reinterpret_cast<char *>(phone), sizeof(short) * 10);

		char *utf8 = 0;
		char **pUtf8 = &utf8;
		if (-1 == convert.Big5toUTF8((char*) big5, byte, pUtf8))
			return;

		int i;
		int nIndex = 0;
		char s[20];
		CString strppt[10];

		memset(s, 0, sizeof(s));
		for (i = 0; i < 10; ++i)
		{
			strppt[i] = "";
		}

		int nLen = utf8len(utf8);

		for (nIndex = 0; nIndex < nLen; ++nIndex)
		{
			memset(s, 0, sizeof(s));
			SID2Phone(phone[nIndex], &s[0]);
			strppt[nIndex] = Phone2Ph97(s, phone[nIndex] % 10);
		}

		strData.format("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", utf8, strppt[0].getBuffer(), strppt[1].getBuffer(),
				strppt[2].getBuffer(), strppt[3].getBuffer(), strppt[4].getBuffer(), strppt[5].getBuffer(),
				strppt[6].getBuffer(), strppt[7].getBuffer(), strppt[8].getBuffer(), strppt[9].getBuffer());

		free(utf8);
		csWordFile1 << strData.getBuffer() << endl;

		_log("%s", strData.getBuffer());
	}

	cf.close();
	csWordFile1.close();

}

void CTextProcess::dumpWordIndex()
{
	ofstream csWordFile("dumpWordIndex.txt", ios::app);
	ifstream cf("model/WORD.NDX", std::ifstream::binary);
	cf.seekg(0, cf.end);
	int fend = cf.tellg();
	cf.seekg(0, cf.beg);
	_log("word index size=%d", fend);
	unsigned short value;
	CString strData;

	while (cf.tellg() != fend)
	{
		cf.read(reinterpret_cast<char *>(&value), sizeof(unsigned short));
		strData.format("%d", value);
		csWordFile << strData.getBuffer() << endl;
		_log("%s", strData.getBuffer());
	}

	cf.close();
	csWordFile.close();
}

void CTextProcess::dumpPhone()
{
	ofstream csWordFile("dumpPhone.txt", ios::app);
	ifstream cf("model/PHONE.DAT", std::ifstream::binary);
	cf.seekg(0, cf.end);
	int fend = cf.tellg();
	cf.seekg(0, cf.beg);
	_log("word index size=%d", fend);
	unsigned short value;
	CString strData;

	while (cf.tellg() != fend)
	{
		cf.read(reinterpret_cast<char *>(&value), sizeof(short));
		strData.format("%d", value);
		csWordFile << strData.getBuffer() << endl;
		_log("%s", strData.getBuffer());
	}

	cf.close();
	csWordFile.close();
}
void CTextProcess::WordExchange(CString &strText)
{
	for (vector<map<std::string, std::string> >::iterator i = vecMaps.begin(); vecMaps.end() != i; ++i)
	{
		for (map<string, string>::iterator j = i->begin(); j != i->end(); ++j)
		{
			size_t start_pos = strText.find((*j).first.c_str());
			if (start_pos != std::string::npos)
			{
				strText = strText.toString().replace(start_pos, (*j).first.length(), (*j).second.c_str());
			}
		}
	}
}
