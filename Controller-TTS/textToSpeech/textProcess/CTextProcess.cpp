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
#include "hts_engine.h"
#include "CConvert.h"
#include "utility.h"
#include "WordInfo.h"
#include "utf8.h"
#include <map>
#include "CController.h"
#include "WaveFile.h"
#include "dirent.h"
#include <spawn.h>
#include <wait.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <regex>

//*****for domain socket client*****//
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
char *socket_path = "/data/opt/ipc_tmp/ipc_tmp.txt";
//***************************//

using namespace std;

#define FEATURE_DIM				14
#define CLUSTER						2
#define CART_MODEL				"model/CART_Model.bin"
#define CART_MODEL2			"model/CART_Model2.bin"
//#define HMM_MODEL				"model/hmm.htsvoice"
#define HMM_MODEL				"model/hmm_adapt.htsvoice"
#define HMM_MODEL2				"model/hmm_original.htsvoice"
#define WORD_MODEL			"model/"
#define PATH_WAVE					"/data/opt/tomcat/webapps/tts/"

#define GEN_PATH            "/data/opt/tomcat/webapps/genlabel"
#define GEN_WAV_PATH 		"/data/opt/tomcat/webapps/genlabel/wav/"
#define GEN_TEXT_PATH       "/data/opt/tomcat/webapps/genlabel/txt/"
#define _MAX_PATH   260
#define max_size 1000

#define Label_PATH         "/data/opt/tomcat/webapps/label/"
#define Data_PATH         "/data/opt/tomcat/webapps/"
#define LabelRow_PATH      "labelrow/"
//#define bin_PATH  		   "/home/kris/ControllerPlatform/Controller-TTS/bin/"       //modified for different user
#define bin_PATH  	       "/data/opt/ControllerPlatform/Controller-TTS/bin/"       //for tts server

static std::map<int, const char*> ModelMap = {
		{0,   "model/hmm_adapt.htsvoice"},
		{1,   "model/hmm_adapt.htsvoice"},
		{2,   "model/hmm_adapt.htsvoice"},
		{3,   "model/hmm_adapt.htsvoice"},
		{9,   "model/hmm_9.htsvoice"},
		{10,  "model/hmm_10.htsvoice"},
		{11,  "model/hmm_11.htsvoice"},
		{12,  "model/hmm_12.htsvoice"},
		{13,  "model/hmm_13.htsvoice"},
		{14,  "model/hmm_14.htsvoice"},
		{15,  "model/hmm_15.htsvoice"},
		{16,  "model/hmm_16.htsvoice"},
		{17,  "model/hmm_17.htsvoice"},
		{18,  "model/hmm_18.htsvoice"},
		{19,  "model/hmm_19.htsvoice"},
		{20,  "model/hmm_20.htsvoice"},
		{21,  "model/hmm_21.htsvoice"},
		{22,  "model/hmm_22.htsvoice"},
		{23,  "model/hmm_23.htsvoice"},
		{24,  "model/hmm_24.htsvoice"},
		{25,  "model/hmm_25.htsvoice"},
		{26,  "model/hmm_26.htsvoice"},
		{101, "model/hmm_101.htsvoice"},
		{102, "model/hmm_102.htsvoice"},
		{103, "model/hmm_103.htsvoice"},
		{104, "model/hmm_104.htsvoice"}
};

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

static std::map<std::string, std::string> splitBilingual = create_map<std::string, std::string>("。", " ")("。", " ")(
		"。", " ")("？", " ")("！", " ")("；", " ")("，", " ")("!", " ")(",", " ")(".", " ")(";", " ")("?", " ")("，", " ")
		("！", " ")("，", " ")("\r", " ")(":", " ")("：", " ")("、", " ")("	", " ")("\\r", " ")("\\n", " ")("\n", " ");


//static std::vector<std::map<std::string, std::string> > splitBilingual =
//		{ Exchange1};

CTextProcess::CTextProcess() :
		CartModel(new CART()), convert(new CConvert), word(new CWord)
{
	loadModel();
	_log("CTextProcess::CTextProcess() :.......");
}

CTextProcess::~CTextProcess()
{
	releaseModel();
	_log("CTextProcess::~CTextProcess().......");
}

void CTextProcess::loadModel()
{
	CartModel->LoadCARTModel();
	word->InitWord(WORD_MODEL);

	FILE *file;
	file = fopen("/data/opt/tomcat/webapps/data/tempWordDataUrl.txt", "r");
	if (file){
		char mystring [100];
		char *a = fgets(mystring, 100, file);
		string str(a);
		word->InitWordfromHTTP(str.c_str());
		fclose(file);
	}

}

int CTextProcess::loadWordfromHTTP(string url)
{
	word->InitWordfromHTTP(url.c_str());
	return 0;
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

vector<string> CTextProcess::splitSentence(string &input){
	vector<string> wordData;
	string strChar = "";
	string blank = " ";
	string reSentence = "";
	string checkSymbolEn = "";
	string checkSymbolCh = "";
	string checkBlankEn;
	string checkBlankCh;
	string splitWordEn = "";
	string splitWordCh = "";
	regex pattern("[A-Za-z]");
	regex patternCh("\，|\。|\！|\：|\；|\“|\”|\（|\）|\、|\？|\《|\ 》");


	//TODO: 將標點符號轉換成空白
	for(int i = 0; input[i] != '\0';){
		char chr = input[i];
		if((chr & 0x80) == 0)  //english
		{
			checkSymbolEn = input.substr(i,1);
			if(!regex_match(checkSymbolEn, pattern)){
				checkSymbolEn = blank;
			}
			reSentence += checkSymbolEn;
			++i;
		}else if((chr & 0xE0) == 0xE0)  //chinese
		{
			checkSymbolCh = input.substr(i,3);
			if(regex_match(checkSymbolCh, patternCh)){
				checkSymbolCh = blank;
			}
			reSentence += checkSymbolCh;
			i+=3;
		}
	}
	_log("[CTextProcess] processTheText reSentence: %s", reSentence.c_str());

	//TODO: 中英文句斷詞
	for(int i = 0; reSentence[i] != '\0';)
	{
		char chr = reSentence[i];
		//英文 chr是0xxx xxxx，即ascii碼
		if((chr & 0x80) == 0)
		{
			strChar = reSentence.substr(i,1);
			checkBlankEn = reSentence.substr(i+1, 1);
			++i;
			if(!strChar.empty() && strChar != blank){
				if(checkBlankEn != blank){
					splitWordEn += strChar;
					if(checkBlankEn == "\0"|| !regex_match(checkBlankEn, pattern)){
						wordData.push_back(splitWordEn);
						splitWordEn = "";
					}
				}else{
					splitWordEn += strChar;
					wordData.push_back(splitWordEn);
					splitWordEn = "";
				}
			}
		}//中文 chr是111x xxxx
		else if((chr & 0xE0) == 0xE0)
		{
			strChar = reSentence.substr(i, 3);
			checkBlankCh = reSentence.substr(i+3, 1);
			i+=3;
			if(!strChar.empty() && strChar != blank){
				if(checkBlankCh != blank){
					splitWordCh += strChar;
					if(checkBlankCh == "\0" || regex_match(checkBlankCh, pattern)){
						wordData.push_back(splitWordCh);
						splitWordCh = "";
					}
				}else{
					splitWordCh += strChar;
					wordData.push_back(splitWordCh);
					splitWordCh = "";
				}
			}
		}
	}

	for (int i = 0; i < wordData.size(); ++i)
	{
		_log("[CTextProcess] processTheText wordData vector: %s", wordData.at(i).c_str());
	}
	return wordData;
}


int CTextProcess::processTheText(TTS_REQ &ttsProcess, CString &strWavePath, CString &strLabelZip, CString &strChineseData)
{

	time_t rawtime;
	time(&rawtime);

	CString LabelRowZip;
	CString strLabelRowFile;
	CString strLabelRow;
	CString strLabelName;
	CString LabelRowFile;
	CString FinalRowPath;
	CString ChineseLabel;
	CString ChinesePath;
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

	int tempcount = 1;
	idCount.insert(pair<string, int>(ttsProcess.id.c_str(), tempcount));
	strWavePath.format("%s%ld.wav", PATH_WAVE, rawtime);
	strLabelName.format("label/%ld.lab", rawtime);
	strLabelRowFile.format("labelrow/%s", ttsProcess.id.c_str());
	LabelRowFile.format("%slabelrow/%s", bin_PATH, ttsProcess.id.c_str());
	LabelRowZip.format("%s.tar.gz", ttsProcess.id.c_str());
	ChineseLabel.format("data/%ld.lab", rawtime);
	AllBig5 = "";

	_log("[CTextProcess] processTheText Input Text: %s", ttsProcess.text.c_str());
	strInput = ttsProcess.text.c_str();
	strInput.trim();

	WordExchange(strInput);
	_log("[CTextProcess] processTheText SpanExcluding and Word Exchange Text: %s", strInput.getBuffer());
	string temp;
	temp = strInput.getBuffer();
	vector<string> splitData = splitSentence(temp);

	string strFinded;
	for (vector<string>::iterator i = splitData.begin(); i != splitData.end(); ++i){
		strFinded = strFinded + *i + " ";
	}
	_log("[CTextProcess] string: %s", strFinded.c_str());

	return 0;
//	string strFinded;
//	while ((i = strInput.findOneOf(vWordWrap, strFinded)) != -1)
//	{
//		for (vector<string>::iterator vdel_it = vWordDel.begin(); vWordDel.end() != vdel_it; ++vdel_it)
//		{
//			strInput.left(i).replace(vdel_it->c_str(), "");
//		}
//		SentenceArray.add(strInput.left(i));
//		strInput = strInput.right(strInput.getLength() - i - strFinded.length());
//	}
//
//	if (0 >= SentenceArray.getSize() || 0 < strInput.getLength())strInput
//	{
//		SentenceArray.add(strInput);
//	}
//
//	for (i = 0; i < SentenceArray.getSize(); ++i)
//	{
//		_log("[CTextProcess] processTheText Sentence: %s", SentenceArray[i].getBuffer());
//	}
//
//	if (ttsProcess.voice_id == -1 && ttsProcess.sequence_num == 1){
//		mkdir(strLabelRowFile, 0777);
//	}
//
//	for (int lcount = 0; lcount < (int) SentenceArray.getSize(); ++lcount)
//	{
//		strLabelRow.format("%s/%ld_%d.lab", strLabelRowFile.getBuffer(), rawtime, idCount[ttsProcess.id.c_str()]);
//
//		CStringArray PhoneSeq;	// 紀錄整個utterance的phone model sequence  音節  音素
//		CString strBig5;
//		vector<int> PWCluster;
//		vector<int> PPCluster;
//
//		_log("initial boundaries and indexes for a sentence");
//		sIndex = wIndex = pIndex = 0;
//		SyllableBound[0] = WordBound[0] = PhraseBound[0] = -1;
//		for (i = 1; i < 100; ++i)
//			SyllableBound[i] = WordBound[i] = 0;
//		for (i = 1; i < 20; ++i)
//			PhraseBound[i] = 0;
//
//		wordPackage.clear();
//		wordPackage.strText = SentenceArray[lcount].getBuffer();
//		wordPackage.txt_len = utf8len(wordPackage.strText.c_str());
//
//		//----- 2019/03/04 fix multiple symbols error -----//
//		_log("[CTextProcess] Word = %s, Number = %d",wordPackage.strText.c_str(), wordPackage.txt_len);
//		if(wordPackage.txt_len == 0){
//			continue;
//		}
//		//-------------------------------------------------//
//
//		word->GetWord(wordPackage);
//
//		if (-1 == CartPrediction(SentenceArray[lcount], strBig5, PWCluster, PPCluster, wordPackage))
//			continue;
//		AllBig5 += strBig5;
//
//		k = l = 0;
//		for (i = 0; i < wordPackage.wnum; ++i) // 字詞數
//		{
//			for (j = 0; j < wordPackage.vecWordInfo[i].wlen; ++j)   // 詞的字數
//			{
//				_log("[CTextProcess] processTheText Text: %s Phone: %s", wordPackage.vecWordInfo[i].strSentence.c_str(),
//						wordPackage.vecWordInfo[i].strPhone[j].c_str());
//
//				vector<string> vData;
//				spliteData(const_cast<char *>(wordPackage.vecWordInfo[i].strPhone[j].c_str()), " ", vData);
//
//				for (vector<string>::iterator it = vData.begin(); vData.end() != it; ++it)  // 因素分解
//				{
//					PhoneSeq.add(*it);
//				}
//				++sIndex;
//
//				SyllableBound[sIndex] = PhoneSeq.getSize() - 1;
//				//_log("======================> SyllableBound[%d] = %d", sIndex, SyllableBound[sIndex]);
//
//				if (!PWCluster.empty() && PWCluster[k] == 1)
//				{
//					WordBound[++wIndex] = SyllableBound[sIndex];
//					if (PPCluster[l] == 2)
//						PhraseBound[++pIndex] = WordBound[wIndex];
//					++l;
//				}
//				++k;
//			}
//		}
//		if(ttsProcess.voice_id == -1){
//			_log("=============== 合成Label檔 %s===============", strLabelRow.getBuffer());
//			ofstream csLabFile;
//			csLabFile.open(strLabelRow.getBuffer(), ios::app);
//			CString full = GenerateLabelFile(PhoneSeq, SyllableBound, WordBound, PhraseBound, sIndex, wIndex, pIndex, csLabFile, NULL,
//					gduration_s, gduration_e,ttsProcess.voice_id);
//			csLabFile.close();
//			PhoneSeq.removeAll();
//			(idCount[ttsProcess.id.c_str()])++;
//
//		}else if(ttsProcess.voice_id == -2){
//			_log("=============== 注音用Label檔 ===============");
//			ofstream csLabFile;
//			csLabFile.open(ChineseLabel.getBuffer(), ios::app);
//			GenerateLabelFile(PhoneSeq, SyllableBound, WordBound, PhraseBound, sIndex, wIndex, pIndex, csLabFile, NULL,
//					gduration_s, gduration_e, ttsProcess.voice_id);
//			csLabFile.close();
//			PhoneSeq.removeAll();
//
//		}else{
//			_log("=============== 合成音標檔 %s===============", strLabelName.getBuffer());
//			ofstream csLabFile;
//			csLabFile.open(strLabelName.getBuffer(), ios::app);
//			GenerateLabelFile(PhoneSeq, SyllableBound, WordBound, PhraseBound, sIndex, wIndex, pIndex, csLabFile, NULL,
//					gduration_s, gduration_e, ttsProcess.voice_id);
//			csLabFile.close();
//			PhoneSeq.removeAll();
//			_log("[CTextProcess] processTheText AllBig5: %s", AllBig5.getBuffer());
//			_log("=============== 合成聲音檔 %s===============",
//					strWavePath.getBuffer());
//			_log("[CTextProcess] Voice_ID: %d", ttsProcess.voice_id);
//			_log("[CTextProcess] Model: %s", ModelMap[ttsProcess.voice_id]);
//			return Synthesize(ModelMap[ttsProcess.voice_id], strWavePath.getBuffer(),
//					strLabelName.getBuffer(), ttsProcess);
//		}
	}

//int CTextProcess::processTheText(TTS_REQ &ttsProcess, CString &strWavePath, CString &strLabelZip, CString &strChineseData)
//{
//
//	time_t rawtime;
//	time(&rawtime);
//
//	CString LabelRowZip;
//	CString strLabelRowFile;
//	CString strLabelRow;
//	CString strLabelName;
//	CString LabelRowFile;
//	CString FinalRowPath;
//	CString ChineseLabel;
//	CString ChinesePath;
//	int nSypollCount;
//	int nIndex;
//	int nLen = 0;
//	int nCount = 0;
//	CString strInput;
//	CString strPart;
//	char s[10];
//	int SyllableBound[100];		// syllable邊界
//	int WordBound[100];			// word boundary, tts 斷詞結果
//	int PhraseBound[20];		// phrase boundary
//	int *gduration_s = NULL;
//	int *gduration_e = NULL;
//	int giSftIdx = 0;
//	int playcount = 0;
//	int i, j, k, l, sIndex, wIndex, pIndex;
//	CStringArray SentenceArray;
//	CString AllBig5;
//	int textNdx;
//	WORD_PACKAGE wordPackage;
//
//	int tempcount = 1;
//	idCount.insert(pair<string, int>(ttsProcess.id.c_str(), tempcount));
//	strWavePath.format("%s%ld.wav", PATH_WAVE, rawtime);
//	strLabelName.format("label/%ld.lab", rawtime);
//	strLabelRowFile.format("labelrow/%s", ttsProcess.id.c_str());
//	LabelRowFile.format("%slabelrow/%s", bin_PATH, ttsProcess.id.c_str());
//	LabelRowZip.format("%s.tar.gz", ttsProcess.id.c_str());
//	ChineseLabel.format("data/%ld.lab", rawtime);
//	AllBig5 = "";
//
//	_log("[CTextProcess] processTheText Input Text: %s", ttsProcess.text.c_str());
//	strInput = ttsProcess.text.c_str();
//	strInput.trim();
//
//	WordExchange(strInput);
//	_log("[CTextProcess] processTheText SpanExcluding and Word Exchange Text: %s", strInput.getBuffer());
//
//	string strFinded;
//	while ((i = strInput.findOneOf(vWordWrap, strFinded)) != -1)
//	{
//		for (vector<string>::iterator vdel_it = vWordDel.begin(); vWordDel.end() != vdel_it; ++vdel_it)
//		{
//			strInput.left(i).replace(vdel_it->c_str(), "");
//		}
//		SentenceArray.add(strInput.left(i));
//		strInput = strInput.right(strInput.getLength() - i - strFinded.length());
//	}
//
//	if (0 >= SentenceArray.getSize() || 0 < strInput.getLength())
//	{
//		SentenceArray.add(strInput);
//	}
//
//	for (i = 0; i < SentenceArray.getSize(); ++i)
//	{
//		_log("[CTextProcess] processTheText Sentence: %s", SentenceArray[i].getBuffer());
//	}
//
//	if (ttsProcess.voice_id == -1 && ttsProcess.sequence_num == 1){
//		mkdir(strLabelRowFile, 0777);
//	}
//
//	for (int lcount = 0; lcount < (int) SentenceArray.getSize(); ++lcount)
//	{
//		strLabelRow.format("%s/%ld_%d.lab", strLabelRowFile.getBuffer(), rawtime, idCount[ttsProcess.id.c_str()]);
//
//		CStringArray PhoneSeq;	// 紀錄整個utterance的phone model sequence  音節  音素
//		CString strBig5;
//		vector<int> PWCluster;
//		vector<int> PPCluster;
//
//		_log("initial boundaries and indexes for a sentence");
//		sIndex = wIndex = pIndex = 0;
//		SyllableBound[0] = WordBound[0] = PhraseBound[0] = -1;
//		for (i = 1; i < 100; ++i)
//			SyllableBound[i] = WordBound[i] = 0;
//		for (i = 1; i < 20; ++i)
//			PhraseBound[i] = 0;
//
//		wordPackage.clear();
//		wordPackage.strText = SentenceArray[lcount].getBuffer();
//		wordPackage.txt_len = utf8len(wordPackage.strText.c_str());
//
//		//----- 2019/03/04 fix multiple symbols error -----//
//		_log("[CTextProcess] Word = %s, Number = %d",wordPackage.strText.c_str(), wordPackage.txt_len);
//		if(wordPackage.txt_len == 0){
//			continue;
//		}
//		//-------------------------------------------------//
//
//		word->GetWord(wordPackage);
//
//		if (-1 == CartPrediction(SentenceArray[lcount], strBig5, PWCluster, PPCluster, wordPackage))
//			continue;
//		AllBig5 += strBig5;
//
//		k = l = 0;
//		for (i = 0; i < wordPackage.wnum; ++i) // 字詞數
//		{
//			for (j = 0; j < wordPackage.vecWordInfo[i].wlen; ++j)   // 詞的字數
//			{
//				_log("[CTextProcess] processTheText Text: %s Phone: %s", wordPackage.vecWordInfo[i].strSentence.c_str(),
//						wordPackage.vecWordInfo[i].strPhone[j].c_str());
//
//				vector<string> vData;
//				spliteData(const_cast<char *>(wordPackage.vecWordInfo[i].strPhone[j].c_str()), " ", vData);
//
//				for (vector<string>::iterator it = vData.begin(); vData.end() != it; ++it)  // 因素分解
//				{
//					PhoneSeq.add(*it);
//				}
//				++sIndex;
//
//				SyllableBound[sIndex] = PhoneSeq.getSize() - 1;
//				//_log("======================> SyllableBound[%d] = %d", sIndex, SyllableBound[sIndex]);
//
//				if (!PWCluster.empty() && PWCluster[k] == 1)
//				{
//					WordBound[++wIndex] = SyllableBound[sIndex];
//					if (PPCluster[l] == 2)
//						PhraseBound[++pIndex] = WordBound[wIndex];
//					++l;
//				}
//				++k;
//			}
//		}
//		if(ttsProcess.voice_id == -1){
//			_log("=============== 合成Label檔 %s===============", strLabelRow.getBuffer());
//			ofstream csLabFile;
//			csLabFile.open(strLabelRow.getBuffer(), ios::app);
//			CString full = GenerateLabelFile(PhoneSeq, SyllableBound, WordBound, PhraseBound, sIndex, wIndex, pIndex, csLabFile, NULL,
//					gduration_s, gduration_e,ttsProcess.voice_id);
//			csLabFile.close();
//			PhoneSeq.removeAll();
//			(idCount[ttsProcess.id.c_str()])++;
//
//		}else if(ttsProcess.voice_id == -2){
//			_log("=============== 注音用Label檔 ===============");
//			ofstream csLabFile;
//			csLabFile.open(ChineseLabel.getBuffer(), ios::app);
//			GenerateLabelFile(PhoneSeq, SyllableBound, WordBound, PhraseBound, sIndex, wIndex, pIndex, csLabFile, NULL,
//					gduration_s, gduration_e, ttsProcess.voice_id);
//			csLabFile.close();
//			PhoneSeq.removeAll();
//
//		}else{
//			_log("=============== 合成音標檔 %s===============", strLabelName.getBuffer());
//			ofstream csLabFile;
//			csLabFile.open(strLabelName.getBuffer(), ios::app);
//			GenerateLabelFile(PhoneSeq, SyllableBound, WordBound, PhraseBound, sIndex, wIndex, pIndex, csLabFile, NULL,
//					gduration_s, gduration_e, ttsProcess.voice_id);
//			csLabFile.close();
//			PhoneSeq.removeAll();
//			_log("[CTextProcess] processTheText AllBig5: %s", AllBig5.getBuffer());
//			_log("=============== 合成聲音檔 %s===============",
//					strWavePath.getBuffer());
//			_log("[CTextProcess] Voice_ID: %d", ttsProcess.voice_id);
//			_log("[CTextProcess] Model: %s", ModelMap[ttsProcess.voice_id]);
//			return Synthesize(ModelMap[ttsProcess.voice_id], strWavePath.getBuffer(),
//					strLabelName.getBuffer(), ttsProcess);
//		}
//	}

//	if(ttsProcess.voice_id == -2){
//		ChinesePath.format("%s%s", bin_PATH, ChineseLabel.getBuffer());
//		char *ChineseFilePath = ChinesePath.getBuffer();
//		char cmd3[] = "mv";
//		char endRoot2[] = "/data/opt/tomcat/webapps/data/";				//mkdir label for different user
//		char *args2[4];
//		args2[0] = cmd3;
//		args2[1] = ChineseFilePath;
//		args2[2] = endRoot2;
//		args2[3] = NULL;
//		pid_t pid3;
//		int status3;
//		status3 = posix_spawn(&pid3, "/bin/mv", NULL, NULL, args2, environ);
//		if (status3 == 0) {
//		 _log("mv: Child pid: %d\n", pid3);
//		 if (waitpid(pid3, &status3, 0) != -1) {
//		  _log("mv: Child exited with status %i\n", status3);
//		 } else {
//		  _log("Error: waitpid");
//		 }
//		} else {
//		 _log("mv: posix_spawn: %s\n", strerror(status3));
//		}
//		strChineseData.format("%s%s", Data_PATH, ChineseLabel.getBuffer());
//		return 0;
//	}
//
//	if(ttsProcess.voice_id == -1 && ttsProcess.sequence_num != ttsProcess.total){
//		return 0;
//	}else if(ttsProcess.voice_id == -1 && ttsProcess.sequence_num == ttsProcess.total){
//
//		char *LabelFileZipPath = strLabelRowFile.getBuffer();
//		char cmd[] = "tar";
//		char param[] = "zcvf";
//		char *LabelFileZipName = LabelRowZip.getBuffer();
//		char *args[5];
//		args[0] = cmd;
//		args[1] = param;
//		args[2] = LabelFileZipName;
//		args[3] = LabelFileZipPath;
//		args[4] = NULL;
//		pid_t pid;
//		int status;
//		status = posix_spawn(&pid, "/bin/tar", NULL, NULL, args, environ);
//
//		if (status == 0) {
//		 _log("tar: Child pid: %d\n", pid);
//		 if (waitpid(pid, &status, 0) != -1) {
//		  _log("tar: Child exited with status %i\n", status);
//		 } else {
//		  _log("Error: waitpid");
//		 }
//		} else {
//		 _log("tar: posix_spawn: %s\n", strerror(status));
//		}
//
//		FinalRowPath.format("%s%s.tar.gz", bin_PATH, ttsProcess.id.c_str());
//		char *LabelFileZipPath2 = FinalRowPath.getBuffer();
//		char cmd2[] = "mv";
//		char endRoot[] = "/data/opt/tomcat/webapps/label/";				//mkdir label for different user
//		char *args2[4];
//		args2[0] = cmd2;
//		args2[1] = LabelFileZipPath2;
//		args2[2] = endRoot;
//		args2[3] = NULL;
//		pid_t pid2;
//		int status2;
//		status2 = posix_spawn(&pid2, "/bin/mv", NULL, NULL, args2, environ);
//		if (status2 == 0) {
//		 _log("mv: Child pid: %d\n", pid2);
//		 if (waitpid(pid2, &status2, 0) != -1) {
//		  _log("mv: Child exited with status %i\n", status2);
//		 } else {
//		  _log("Error: waitpid");
//		 }
//		} else {
//		 _log("mv: posix_spawn: %s\n", strerror(status2));
//		}
//		strLabelZip.format("%s%s.tar.gz", Label_PATH, ttsProcess.id.c_str());
//		idCount.erase(ttsProcess.id.c_str());
//		return 0;
//	} else {
//		_log("[CTextProcess] processTheText AllBig5: %s", AllBig5.getBuffer());
//		_log("=============== 合成聲音檔 %s===============",
//				strWavePath.getBuffer());
//		_log("[CTextProcess] Voice_ID: %d", ttsProcess.voice_id);
//		_log("[CTextProcess] Model: %s", ModelMap[ttsProcess.voice_id]);
//		return Synthesize(ModelMap[ttsProcess.voice_id], strWavePath.getBuffer(),
//				strLabelName.getBuffer(), ttsProcess);
//	}
//}


void CTextProcess::genLabels(){

	CString strTextTitle2;
	CString strLabelName;
	int SyllableBound[100];		// syllable邊界
	int WordBound[100];			// word boundary, tts 斷詞結果
	int PhraseBound[20];		// phrase boundary
	int i, j, k, l, sIndex, wIndex, pIndex;
	CStringArray SentenceArray;
	CString AllBig5;
	WORD_PACKAGE wordPackage;
	initrd();
	CWaveFile WaveFile;  // class for loading cue information from wav file
	CString csTargetWavName;  // filename of wav file with absolute path
	CString csTargetTxtName;  // filename of wav file with absolute path

	csTargetWavName.format("%s%s", GEN_WAV_PATH ,strFileTitle_gen.getBuffer());
	char *temp = 0;
	temp = csTargetWavName.getBuffer();
	_log("[CTextProcess]path %s\n", temp);

	ifstream cf(temp, std::ifstream::binary);
	cf.seekg(0, cf.beg);
	unsigned long totalcues;
	unsigned long data;
	char head[40];
	char shortdata[5];
	char* wavebuffer = NULL;
	cf.read(reinterpret_cast<char*>(&head), 40);			//位址 0
	cf.read(reinterpret_cast<char*>(&data), 4);				//位址 40 , 值為 資料長度

	wavebuffer = new char[data];
	cf.read(wavebuffer, data);		// read data into wavebuffer
	free(wavebuffer);
	wavebuffer = NULL;
//	printf("data :%d\n", data);

	cf.read(shortdata, 4);			//開始 read 最後 cue 資料
	shortdata[4] = '\0';
	//先檢查是否有 cue 資料
	if (strcmp(shortdata, "cue ") != 0) {
		cf.close();
	} else {
		cf.seekg(4, cf.cur);
		cf.read(reinterpret_cast<char*>(&totalcues), 4);// how many cues in the wave file
	}

	int nTotalCue = (unsigned int) totalcues;// total # cue tags in the wav file, including tags of all sentences.
//	printf("total %d\n", nTotalCue);
//	unsigned long data2;
	char head2[40];
	char *wavebuffer2 = NULL;
	ifstream cf2(temp, std::ifstream::binary);
	free(temp);
	cf2.seekg(0, cf2.beg);
	cf2.read(reinterpret_cast<char*>(&head2), 40);
	cf2.read(reinterpret_cast<char*>(&data), 4);
	wavebuffer2 = new char[data];
	cf2.read(wavebuffer2, data);
//	printf("data2 :%d\n", data2);
	free(wavebuffer2);
	wavebuffer2 = NULL;

	UINT *cuestart = new UINT[nTotalCue];		//cue start
	UINT *cuelength = new UINT[nTotalCue];		//cue length
	UINT *cuestart2 = new UINT[nTotalCue + 1];	//cue start
	UINT *cuelength2 = new UINT[nTotalCue + 1];	//cue length

	cf2.seekg(12, cf2.cur);
	for ( int i = 0; i < nTotalCue; i++) {
		cf2.seekg(4, cf2.cur);
		cf2.read(reinterpret_cast<char*>(&data), 4);
		cf2.seekg(16, cf2.cur);
		cuestart[i] = (unsigned int) data;
	}
	cf2.seekg(12, cf2.cur);

	for ( int j = 0; j < nTotalCue; j++) {
		cf2.seekg(12, cf2.cur);
		cf2.read(reinterpret_cast<char*>(&data), 4);
		cf2.seekg(12, cf2.cur);
		cuelength[j] = (unsigned int) data;
	}

	for (int i = 0; i < nTotalCue; i++) { // convert unit to sample-point based.
		cuestart2[i + 1] = cuestart[i] * 1000 / 1.6;
		cuelength2[i + 1] = cuestart2[i + 1] + cuelength[i] * 1000 / 1.6;

	}

	strInput_gen.trim();
	WordExchange(strInput_gen);
	_log("[CTextProcess] genlabel processTheText SpanExcluding and Word Exchange Text: %s", strInput_gen.getBuffer());

	string strFinded;
	while ((i = strInput_gen.findOneOf(vWordWrap, strFinded)) != -1)
	{
		for (vector<string>::iterator vdel_it = vWordDel.begin(); vWordDel.end() != vdel_it; ++vdel_it)
		{
			strInput_gen.left(i).replace(vdel_it->c_str(), "");
		}
		SentenceArray.add(strInput_gen.left(i));
		strInput_gen = strInput_gen.right(strInput_gen.getLength() - i - strFinded.length());
	}

	if (0 >= SentenceArray.getSize() || 0 < strInput_gen.getLength())
	{
		SentenceArray.add(strInput_gen);
	}

	for (i = 0; i < SentenceArray.getSize(); ++i)
	{
		_log("[CTextProcess] genlabel processTheText Sentence: %s", SentenceArray[i].getBuffer());
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

		_log("[CTextProcess] genlabel Word = %s, Number = %d",wordPackage.strText.c_str(), wordPackage.txt_len);
		if(wordPackage.txt_len == 0){
			continue;
		}
		word->GetWord(wordPackage);

		if (-1 == CartPrediction(SentenceArray[lcount], strBig5, PWCluster, PPCluster, wordPackage))
			continue;
		AllBig5 += strBig5;

		k = l = 0;
		for (i = 0; i < wordPackage.wnum; ++i) // 字詞數
		{
			for (j = 0; j < wordPackage.vecWordInfo[i].wlen; ++j)   // 詞的字數
			{
				_log("[CTextProcess] genlabel processTheText Text: %s Phone: %s", wordPackage.vecWordInfo[i].strSentence.c_str(),
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
		ofstream csLabFileFull;
		ofstream csLabFileMono;
		CString strFileName;
		FinalFileTitle = strFileTitle_gen.getBuffer();
		string wav = ".wav";
		FinalFileTitle = FinalFileTitle.replace(FinalFileTitle.find(wav), sizeof(wav), "");
		strFileName.format("%s/train/full/temp_%s_%d.lab", GEN_PATH, FinalFileTitle.c_str(), lcount);
		csLabFileFull.open(strFileName.getBuffer(), ios::app);
		strFileName.format("%s/train/mono/temp_%s_%d.lab", GEN_PATH, FinalFileTitle.c_str(), lcount);
		csLabFileMono.open(strFileName.getBuffer(), ios::app);
		timeinfo((int*)cuestart2,(int*)cuelength2);
		GenerateLabelFile(PhoneSeq, SyllableBound, WordBound, PhraseBound, sIndex, wIndex, pIndex, csLabFileFull, &csLabFileMono,
				gduration_s, gduration_e, 1);
		csLabFileFull.close();
		csLabFileMono.close();
	}
	CString strFileName;
	strFileName.format("%s/train", GEN_PATH );
	ConcatenateLabel( FinalFileTitle.c_str(), strFileName.getBuffer(), SentenceArray.getSize() ) ;
	delete [] cuestart;
	delete [] cuelength;
	delete [] cuestart2;
	delete [] cuelength2;

}

void CTextProcess::ConcatenateLabel( string outfilename, char* dir, int iSentenceCnt )
{
	FILE *fpMono;
	FILE *fpFull;
	char output[_MAX_PATH];
	char cBuf[1024] ;
	sprintf(output,"%s/mono/%s.lab",dir,outfilename.c_str());
	fpMono=fopen(output,"w+");
	sprintf(output,"%s/full/%s.lab",dir,outfilename.c_str());
	fpFull=fopen(output,"w+");
	for( int i = 0 ; i < iSentenceCnt ; i++ ){
		sprintf(output,"%s/mono/temp_%s_%d.lab",dir,outfilename.c_str(), i );
		FILE *fp = fopen( output, "r" ) ;
		while( !feof(fp) ) {
			cBuf[0]=0 ;
			fgets(cBuf,1024,fp) ;
			if( strlen(cBuf) == 0 )
				continue ;
			fprintf( fpMono, "%s", cBuf ) ;
		}
		fclose(fp) ;


		sprintf(output,"%s/full/temp_%s_%d.lab",dir,outfilename.c_str(), i);
		fp = fopen( output, "r" ) ;
		while( !feof(fp) ) {
			cBuf[0]=0 ;
			fgets(cBuf,1024,fp) ;
			if( strlen(cBuf) == 0 )
				continue ;
			fprintf( fpFull, "%s", cBuf ) ;
		}
		fclose(fp) ;
	}
	fclose(fpMono) ;
	fclose(fpFull) ;
}

bool CTextProcess::timeinfo(int* duration_si, int* duration_ei)
{
	//	duration_e=duration_ei;
	gduration_e = duration_ei + giSftIdx; // ky modify
	//	duration_s=duration_si;
	gduration_s = duration_si + giSftIdx; // ky modify
	return true;
}

bool CTextProcess::initrd()
{
	gduration_s = NULL;
	gduration_e = NULL;
	giSftIdx = 0 ; // ky add
	return true;
}

int CTextProcess::Synthesize(const char* szModelName, const char* szWaveName, const char* szLabel, TTS_REQ &ttsprocess2)
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
	param[8] = const_cast<char*>("-g");
	param[9] = const_cast<char*>("0.0");
	param[10] = const_cast<char*>("-r");
	param[11] = const_cast<char*>("1.2");

	if(strlen(ttsprocess2.fm.c_str()) != 0){
		param[7] = const_cast<char*>(ttsprocess2.fm.c_str());
		_log("[CTextProcess] fm: %s", param[7]);
	}
	if(strlen(ttsprocess2.g.c_str()) != 0){
		param[9] = const_cast<char*>(ttsprocess2.g.c_str());
		_log("[CTextProcess] g: %s", param[9]);
	}
	if(strlen(ttsprocess2.r.c_str()) != 0){
		param[11] = const_cast<char*>(ttsprocess2.r.c_str());
		_log("[CTextProcess] r: %s", param[11]);
	}
	_log("[CTextProcess] Synthesize Model Name: %s Wave Name: %s Label Name: %s fm: %s g: %s r: %s", szModelName, szWaveName, szLabel, param[7], param[9], param[11]);
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
	if (wordPackage.wnum <= 0) {
		// Avoid calling Stanford Domain Socket Server with empty Socket input. (Ex: " ")
		_log("[CTextProcess] wordPackage.wnum <= 0 :-->return -1");
		return -1;
	}
	_log("[CTextProcess] 928");
    //***** Stanford domain socket connect *****//

//	struct sockaddr_un addr;
//	int fd;
//	char receiveMessage[max_size];
//
//	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
//		perror("socket error");
//		exit(-1);
//	}
//	memset(&addr, 0, sizeof(addr));
//	addr.sun_family = AF_UNIX;
//	if (*socket_path == '\0') {
//		*addr.sun_path = '\0';
//	  strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
//	} else {
//	  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
//	}
//	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
//	  perror("connect error");
//	  exit(-1);
//	}

	//********************************//
	_log("[CTextProcess] 953");
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
		strBig5 += wordPackage.vecWordInfo[i].strSentence;
		cstemp += wordPackage.vecWordInfo[i].strSentence ;
		cstemp = cstemp + "/X" + " ";
//		cstemp = cstemp + " ";
	}

	//***** domain socket *****//

//	memset(receiveMessage, 0, sizeof(receiveMessage));
//	write(fd, cstemp, cstemp.getLength());
//	_log("[CTextProcess] Socket input: %s", cstemp.getBuffer());
//	read(fd,receiveMessage,sizeof(receiveMessage));
//	_log("[CTextProcess] Socket receive: %s", receiveMessage);
//	CString cstring = receiveMessage;
//	cstemp = cstring;

	//************************//

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
		if (!tempPOS.Compare("X")){      // no pos
			tempPOS.format("%s", "DEC"); // X -> DEC
		}
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
//		_log("cluster push: %d", pcdData->clu);
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
		int *gduration_s, int *gduration_e,int voice_id)
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
//		printf("1_fullstr: %s\n", fullstr.getBuffer());
//		printf("1_monostr: %s\n", monostr.getBuffer());

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
//	printf("2_fullstr: %s\n", fullstr.getBuffer());
//	printf("2_monostr: %s\n", monostr.getBuffer());

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
			_log("1341");

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
//			printf("3_fullstr: %s\n", fullstr.getBuffer());
//			printf("3_monostr: %s\n", monostr.getBuffer());
			tempstr.format("%s\n", sequence[index].getBuffer());
			monostr += tempstr;
//			printf("4_fullstr: %s\n", fullstr.getBuffer());
//			printf("4_monostr: %s\n", monostr.getBuffer());
		}
		//
		_log("1361");
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
		_log("1400");
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
		_log("1466");
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
		_log("1512");

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

	if (gduration_s != NULL)
	{
		int tmp = 0;
		int iCount_2 = sCount;  // simulate index for count[2]
		giSftIdx += sCount; // update global shift of syllable index for each sentence
		tmp = gduration_e[iCount_2] + 1000000;
		tempstr.format("%10d %10d ", gduration_e[iCount_2], tmp);
		fullstr += tempstr;
		monostr += tempstr;
//		monostr += tempstr;
//		printf("5_fullstr: %s\n", fullstr.getBuffer());
//		printf("5_monostr: %s\n", monostr.getBuffer());
	}


	tempstr.format("%s^%s-pau+x=x@x_x/A:%d/B:x@x-x&x-x/C:0", sequence[index - 1].getBuffer(),
			sequence[index].getBuffer(), sBound[sIndex] - sBound[sIndex - 1]);
	fullstr += tempstr;
	tempstr.format("pau\n");  // ky add
	monostr += tempstr; // ky add
//	monostr += tempstr;
//	printf("6_fullstr: %s\n", fullstr.getBuffer());
//	printf("6_monostr: %s\n", monostr.getBuffer());
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

	//-----  filterlabel 2019/03/07-----//
	fullstr = filterLabel(fullstr, voice_id);
//	_log("[CTextProcess] Voice_ID:\n %d, Label: %s", voice_id, fullstr.getBuffer());

	csFile << fullstr;	//fullstr即為輸出的Label內容
	//csFile.close();

	if (voice_id == -2){
		return fullstr;
	}

	if (pcsFile2 != NULL)
	{	// ky add: write out mono labels if needed
		(*pcsFile2) << monostr;
		//*pcsFile2.close();
	}
	monostr += tempstr;
//	printf("7_fullstr:\n %s\n", fullstr.getBuffer());
//	printf("7_monostr:\n %s\n", monostr.getBuffer());fullstr
	return fullstr, monostr;
}

CString CTextProcess::filterLabel(CString fullstr, int voice_id) {  //-----  filterlabel modified 2019/03/20/ -----//
	if (voice_id <= 100)
		return fullstr;
	_log("[CTextProcess] Filter Label: %d", voice_id);
	string strfullstr = fullstr.toString();
	char* labels = strdup(strfullstr.c_str());
	char* SplitLabel;
	CString CStrSplitLabel;
	CString FinalLabel;

	SplitLabel = strtok(labels, "\n");
	FinalLabel = filterLabelLine(SplitLabel);
	while (SplitLabel != NULL) {
		SplitLabel = strtok(NULL, "\n");
		if (SplitLabel != NULL) {
			FinalLabel += filterLabelLine(SplitLabel);
		}
	}
	return FinalLabel;
}

CString CTextProcess::filterLabelLine(char* SplitLabel) { //-----  filterlabel 2019/03/20/ -----//
	string StrSplitLabel;
	CString CStrSplitLabel;
	string empty = "";
	string firstStrSplitLabel;
	CString fisrttempSplitLabel;

	firstStrSplitLabel = SplitLabel;
	int idx_b0 = firstStrSplitLabel.find("/B:", 0);
	int idx_c0 = firstStrSplitLabel.find("/C:", 0);
	int idx_d0 = firstStrSplitLabel.find("/D:", 0);
	int idx_j0 = firstStrSplitLabel.find("/J:", 0);
	string seg_10 = firstStrSplitLabel.substr(idx_b0, (idx_c0 - idx_b0));
	seg_10 = seg_10.substr(seg_10.find("@"));
	string seg_20 = firstStrSplitLabel.substr(idx_d0, (idx_j0 - idx_d0));
	string seg_30 = firstStrSplitLabel.substr(idx_j0);
	seg_30 = seg_30.substr(seg_30.find("+"));
	string firstfinalLabel = firstStrSplitLabel.replace(firstStrSplitLabel.find(seg_10), seg_10.length(), empty);
	firstfinalLabel = firstStrSplitLabel.replace(
			firstStrSplitLabel.find(seg_20), seg_20.length(), empty);
	firstfinalLabel = firstStrSplitLabel.replace(
			firstStrSplitLabel.find(seg_30), seg_30.length(), empty);
	fisrttempSplitLabel.format("%s", firstfinalLabel.c_str());
	CStrSplitLabel = (fisrttempSplitLabel + "\n");
	return CStrSplitLabel;
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
