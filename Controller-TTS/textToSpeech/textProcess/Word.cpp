#include "Word.h"
#include "Phone.h"
#include "CString.h"
#include "LogHandler.h"
#include <cmath>
#include "CConvert.h"
#include <fstream>
#include <stdio.h>
#include "utf8.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <CController.h>   //kris add
#include<iostream>

//#define WORD_FILE "WORD.DAT"
//#define WORD_INDEX_FILE "WORD.NDX"
//#define PHONE_FILE "PHONE.DAT"

using namespace std;

//static USHORT word_index_boundry = 0;
//static unsigned short *word_index = NULL;
//static WORD_DB *word_data = 0;

unsigned char numberic[][4] = { "零", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十", "百", "千", "萬", "億", "兆", "半",
		"幾", "多", "少", "壹", "貳", "參", "肆", "伍", "陸", "柒", "捌", "玖", "拾", "佰", "仟", "廿" };
//static int num = 33;

CWord::CWord()
{

}

CWord::~CWord()
{

}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void CWord::InitWordfromHTTP(string wordDataUrl)
{

	//-----test url
    CURL *curl;
    FILE *fp;
    CURLcode res;
    char error[CURL_ERROR_SIZE];
//  "http://www.more.org.tw/ttsWordData/WordData_linuxOnline_test333.txt";
    const char *url = wordDataUrl.c_str();
    char outfilename[FILENAME_MAX] = "tempWordData.txt"; //存檔路徑
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outfilename,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url); //設定網址
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        //如果有錯誤的話會將錯誤寫在這邊的error buffer
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }
	//-----
	FILE* f;
	CString cs;
	int len;
	//==================== 載入字詞字典檔 =======================//
	ifstream file("tempWordData.txt");
	string str;
	int count2 = 1;
	if (file.is_open())
	{
		char * pch;
		int nIndex;
		mapWordDictionary.clear();
		while (getline(file, str))
		{
			vector<WORD_DIC> vecWord;
			WORD_DIC worddic;
			pch = strtok(const_cast<char*>(str.c_str()), ","); // 抓字
			if (NULL != pch)
			{
				worddic.strWord = pch;
				if (mapWordDictionary.end() == mapWordDictionary.find(utf8_substr(worddic.strWord, 0, 1)))
				{
//					_log("%s\n", utf8_substr(worddic.strWord, 0, 1).c_str());
					mapWordDictionary[utf8_substr(worddic.strWord, 0, 1)] = vecWord;
				}

				pch = strtok( NULL, ",");
				nIndex = 0;
				while (pch != NULL)
				{
					worddic.strPhone[nIndex] = pch;
					pch = strtok( NULL, ",");
					++nIndex;
				}
				mapWordDictionary[utf8_substr(worddic.strWord, 0, 1)].push_back(worddic);
			}
			count2++;
		}
		file.close();
		_log("[key count] %d", mapWordDictionary.size());
		_log("[line count] %d", count2);
#ifdef DEBUG
		for (map<std::string, vector<WORD_DIC> >::iterator it = mapWordDictionary.begin();
				it != mapWordDictionary.end(); ++it)
		{
			_log("<==================== %s =====================>", it->first.c_str());
			vector<WORD_DIC> vecDic = it->second;
			for (vector<WORD_DIC>::iterator vecit = it->second.begin(); it->second.end() != vecit; ++vecit)
			{
				_log("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", vecit->strWord.c_str(), vecit->strPhone[0].c_str(),
						vecit->strPhone[1].c_str(), vecit->strPhone[2].c_str(), vecit->strPhone[3].c_str(),
						vecit->strPhone[4].c_str(), vecit->strPhone[5].c_str(), vecit->strPhone[6].c_str(),
						vecit->strPhone[7].c_str(), vecit->strPhone[8].c_str(), vecit->strPhone[9].c_str());
			}
		}
#endif
	}
	_log("[CWord] InitWordforHTTP success :%s", url);
	FILE* tempWordDataRecord;
	string datapath = "/data/opt/tomcat/webapps/data/tempWordDataUrl.txt";
	tempWordDataRecord = fopen(datapath.c_str(), "w");
	fwrite(wordDataUrl.c_str(), 1, wordDataUrl.size(), tempWordDataRecord);
	fclose(tempWordDataRecord);
}
void CWord::InitWord(LPCTSTR dir)
{
	FILE* f;
	CString cs;
	int len;
	int count = 1;
	//==================== 載入字詞字典檔 =======================//
	ifstream file("model/WordData.txt");
	string str;
	if (file.is_open())
	{
		char * pch;
		int nIndex;

		while (getline(file, str))
		{
			vector<WORD_DIC> vecWord;
			WORD_DIC worddic;
			pch = strtok(const_cast<char*>(str.c_str()), ","); // 抓字
//			_log("[word] pch = %s", pch);
			if (NULL != pch)
			{
				worddic.strWord = pch;
				if (mapWordDictionary.end() == mapWordDictionary.find(utf8_substr(worddic.strWord, 0, 1)))
				{
//					_log("%s\n", utf8_substr(worddic.strWord, 0, 1).c_str());
					string s = utf8_substr(worddic.strWord, 0, 1).c_str();
					mapWordDictionary[utf8_substr(worddic.strWord, 0, 1)] = vecWord;
				}

				pch = strtok( NULL, ",");
				nIndex = 0;
				while (pch != NULL)
				{
					worddic.strPhone[nIndex] = pch;
					pch = strtok( NULL, ",");
					++nIndex;
				}

				mapWordDictionary[utf8_substr(worddic.strWord, 0, 1)].push_back(worddic);
			}
			count++;
		}
		file.close();
//		_log("[total size] %d", mapWordDictionary.size());
//		_log("[total sequence] %d", count);
#ifdef DEBUG
		for (map<std::string, vector<WORD_DIC> >::iterator it = mapWordDictionary.begin();
				it != mapWordDictionary.end(); ++it)
		{
			_log("<==================== %s =====================>", it->first.c_str());
			vector<WORD_DIC> vecDic = it->second;
			for (vector<WORD_DIC>::iterator vecit = it->second.begin(); it->second.end() != vecit; ++vecit)
			{
				_log("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", vecit->strWord.c_str(), vecit->strPhone[0].c_str(),
						vecit->strPhone[1].c_str(), vecit->strPhone[2].c_str(), vecit->strPhone[3].c_str(),
						vecit->strPhone[4].c_str(), vecit->strPhone[5].c_str(), vecit->strPhone[6].c_str(),
						vecit->strPhone[7].c_str(), vecit->strPhone[8].c_str(), vecit->strPhone[9].c_str());
			}
		}
#endif
	}

	_log("[CWord] InitWord success");
}

void CWord::GetWord(WORD_PACKAGE &wordPackage)
{
	int i, j, k, start, offset;
	string strWord;
	string strWordRight;
	string strText;

	//================= UTF-8 Version Start ============================//
	int nTxtLen = wordPackage.strText.length();
	int nWordNum = utf8len(wordPackage.strText.c_str());
	int nDicWordNum;
	int nDicWord;
	int nWordLen;
	int nWnum = 0;
	int offseted = 0;
	map<std::string, vector<WORD_DIC> >::iterator itWordDic;
	vector<WORD_DIC>::iterator itVecDic;
	vector<WORD_DIC> vecDic;
	i = 0;
	strText = wordPackage.strText;
	_log("[CWord] GetWord txt byte = %d word number = %d word: %s", nTxtLen, nWordNum, strText.c_str());

	while (1)
	{
		if (0 >= GetUtf8Word(strText.c_str(), nWnum + 1, offset))
			break;

		nWordLen = offset - offseted;
		strWord = strText.substr(i, nWordLen);
		strWordRight = strText.substr(i);
		offseted = offset;

		//=========== 查字典檔 ===============//
		vecDic = mapWordDictionary[strWord];

		for (itVecDic = vecDic.begin(); vecDic.end() != itVecDic; ++itVecDic)
		{
			//	_log("======= word:%s   dic word: %s phone: %d", strWordRight.c_str(), itVecDic->strWord.c_str(),					itVecDic->phoneID[0]);
			nDicWordNum = nDicWord = 0;
			if (0 == strWordRight.compare(0, itVecDic->strWord.length(), itVecDic->strWord.c_str()))
			{
				_log("===== word: %s   compare ok %s =======================", strWordRight.c_str(),
						itVecDic->strWord.c_str());
				strWordRight = strWordRight.substr(0, itVecDic->strWord.length());
				nDicWord = itVecDic->strWord.length();
				nDicWordNum = utf8len(itVecDic->strWord.c_str());
				WORD_INFO word_info;
				word_info.strSentence = itVecDic->strWord;
				word_info.wlen = nDicWordNum;
				for (int phoneidx = 0; phoneidx < nDicWordNum; ++phoneidx)
				{
					word_info.strPhone[phoneidx] = itVecDic->strPhone[phoneidx];
				}
				wordPackage.vecWordInfo.push_back(word_info);
				++wordPackage.wnum;
				//	_log("======== word right: %s  dicword size: %d  dicwordNum: %d ===========", strWordRight.c_str(),						nDicWord, nDicWordNum);
				break;
			}
		}

		if (0 != nDicWord)
		{
			i = offset + nDicWord - nWordLen;
			nWnum += nDicWordNum;
			offseted += (nDicWord - nWordLen);
		}
		else
		{
			i = offset;
			++nWnum;
		}
		nDicWord = 0;

		if (nWnum > nWordNum)
			break;
	}
}
