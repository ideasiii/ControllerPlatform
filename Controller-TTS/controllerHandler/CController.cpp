/*
 * CController.cpp
 *
 *  Created on: 2018年9月27日
 *      Author: Jugo
 */
#include "CController.h"
#include <stdio.h>
#include <string>
#include <typeinfo>
#include <iostream>
#include <thread>
#include "common.h"
#include "event.h"
#include "CTextProcess.h"
#include "CCmpTTS.h"
#include "CConfig.h"
#include "utility.h"
#include "packet.h"
#include "JSONObject.h"
#include "CString.h"
#include <spawn.h>    //new 2019/03/19
#include <wait.h>
#include <unistd.h>
#include <regex>
#include "WordInfo.h"
#include <spawn.h>
#include <ctype.h>
#include <fstream>
#include <dirent.h>

#define TTS_HOST				"http://175.98.119.122"
#define GEN_PATH            "/data/opt/tomcat/webapps/genlabel"     //kris 2019/04/11 read directory file
#define DATA_PATH			"/data/opt/tomcat/webapps/data/"

#define WAV_PATH            "/data/opt/tomcat/webapps/tts/"

//----------- TODO: 如要genlabel 註解此處 ----------//

#define CHAR_ZERO   CHAR_NUM[0]
const int MAX_LEN = 20;
const int INTERVAL = 4;
// chinese numerals
const string CHAR_NUM[] = { "零", "一", "二", "三", "四", "五", "六", "七", "八", "九" };
// small interval
const string CHAR_SI[] = { "十", "百", "千" };
// big interval
const string CHAR_BI[] = { "萬", "億", "兆", "京" };

const string DictA[] = { "Zero", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten",
		"Eleven", "Twelve", "Thirteen", "Fourteen", "Fifteen", "Sixteen", "Seventeen", "Eighteen", "Nineteen" };
const string DictB[] = { "Zero", "Ten", "Twenty", "Thirty", "Forty", "Fifty", "Sixty", "Seventy", "Eighty", "Ninety" };
const string DictC[] = { "", "Thousand", "Million", "Billion", "Trillion"
		"Thousand Trillion", "Million Trillion", "Billion Trillion" };

//--------------------------------------------------//

using namespace std;

CController::CController() :
		mnMsqKey(0), textProcess(0), cmpTTS(0)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_TTS;
	textProcess = new CTextProcess();
	cmpTTS = new CCmpTTS(this);
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nResult = 0;
	int nCount;
	int nPort;
	string strConfPath;
	string strPort;
	CConfig *config;

	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());

	if (!strConfPath.empty())
	{
		config = new CConfig();
		if (config->loadConfig(strConfPath))
		{
			strPort = config->getValue("SERVER", "port");
			if (!strPort.empty())
			{
				convertFromString(nPort, strPort);
				nResult = cmpTTS->start(0, nPort, mnMsqKey);
////-- standford auto connect not use
//				if (nResult > 0){
//					pid_t pid;
//					char *argv[] = { "sh", "-c", "/data/opt/DomainSocketServer/startup.sh", NULL };
//					int status;
//
//					status = posix_spawn(&pid, "/bin/sh", NULL, NULL, argv, environ);
//					if (status == 0) {
//					 _log("StanfordDomainSocketServer: Child pid: %d\n", pid);
//					 if (waitpid(pid, &status, 0) != -1) {
//					  _log("StanfordDomainSocketServer: Child exited with status %i\n", status);
//					 } else {
//					  _log("Error: StanfordDomainSocketServer: waitpid");
//					 }
//					} else {
//					 _log("StanfordDomainSocketServer: posix_spawn: %s\n", strerror(status));
//					}
//				}
////--
			}

		}
		delete config;
	}
	sleep(2);
//	// gen label ----------

//	CString filepath;
//	CString filename;
//	CString csTargetFileName;
//	string datain;
//
//	filepath.format("%s/wav/",GEN_PATH);
//	_log("[CController]nfile %s\n", filepath.getBuffer());
//
//	DIR *dp;
//	struct dirent *entry;
//	if((dp = opendir(filepath.getBuffer())) == NULL){
//		fprintf(stderr, "cant open %s", filepath.getBuffer());
//		printf("Error open\n");
//	}
//	while ((entry = readdir(dp)) != NULL) {
//		if (entry->d_name[0] == '.')
//			continue;
//		filename = entry->d_name;
//		_log("[CController]fileName: %s", filename.getBuffer());
//		std::string FinalTitle = filename.getBuffer();
//		std::string FinalTitle2 = FinalTitle;
//		string wav = ".wav";
//		FinalTitle = FinalTitle.replace(FinalTitle.find(wav), sizeof(wav), "");
//		csTargetFileName.format("%s/txt/%s.txt",GEN_PATH,FinalTitle.c_str());
//		_log("[CController]csTargetFileName: %s\n", csTargetFileName.getBuffer());
//		ifstream test(csTargetFileName, std::ifstream::in);
//		getline(test,datain);
//		_log("[CController]dataIn : %s", datain.c_str());
//		test.close();
//		textProcess->strFileTitle_gen = FinalTitle2.c_str();
//		textProcess->strInput_gen = datain;
//		textProcess->genLabels();
//
//	}

//	//------------

//textProcess->dumpWordData();
//textProcess->dumpWordIndex();
//textProcess->dumpPhone();
//CString strWav;
//textProcess->processTheText("你在說什麼?多型態角色語音智慧平台，我說一個故事給你們聽。要注意聽!千萬要注意聽，因為；如果沒聽到，你一定會問，你在說什麼?", strWav);
//_log("=====================憋魚酒氣了");
	return nResult;
}

int CController::onFinish(void* nMsqKey)
{
	int nKey = *(reinterpret_cast<int*>(nMsqKey));
	delete textProcess;
	delete cmpTTS;
	return nKey;
}

//----------- TODO: 如要genlabel 註解此處 ----------//

vector<string> CController::parseArticle(string &sentence)
{
	vector<string> articleList;
	string strChar = "";
	string output = "";
	string checkEn = "";
	string checkCh = "";
	regex pattern("[A-Za-z0-9]");
	regex patternCh("[\，|\。|\！|\：|\；|\“|\”|\（|\）|\、|\？|\《|\ 》|\「|\」|\～|\—|\﹏]");
	regex patternBlank("[ ]");

	for (int i = 0; sentence[i] != '\0';)
	{
		char chr = sentence[i];
		if ((chr & 0x80) == 0)
		{
			strChar = sentence.substr(i, 1);
			checkEn = sentence.substr(i + 1, 1);
			++i;
			output += strChar;
			if (checkEn == "\0" || (!regex_match(checkEn, pattern) && !regex_match(checkEn, patternBlank)))
			{
				articleList.push_back(output);
				output = "";
//				++i;
			}
			_log("[CController] processTheText output: %s", output.c_str());
		}
		else if ((chr & 0xE0) == 0xE0)
		{
			strChar = sentence.substr(i, 3);
			checkCh = sentence.substr(i + 3, 1);
			checkEn = sentence.substr(i + 1, 1);
			i += 3;
			output += strChar;
			if (checkCh == "\0" || regex_match(checkCh, patternCh))
			{
				articleList.push_back(output);
				output = "";
//				i+=3;
			}
		}

	}

	for (int i = 0; i < articleList.size(); ++i)
	{
		_log("[CController] processTheText articleList vector: %s", articleList.at(i).c_str());
	}

	return articleList;
}

vector<string> CController::parseSentence(string &sentence)
{
	vector<string> wordData;
	vector<toWord> wordData2;
	_log("[CController] sentence: %s", sentence.c_str());
	wordData2 = toWords(sentence);
	wordData = phase(wordData2);
	return wordData;

}

vector<toWord> CController::toWords(string &sentence)
{
	vector<toWord> wordData;
	toWord toword;
	string strChar = "";
	string splitWordEn = "";
	string checkBlankEn = "";
	regex patternEn("[A-Za-z0-9]");
	regex patternBlank("[ \f\n\r\t\v]");
	string check;
	regex patternNum("[A-Za-z]");
	CString temp;

	string splitWordEn2;
	string test;

	for (int i = 0; sentence[i] != '\0';)
	{
		char chr = sentence[i];
		if ((chr & 0xE0) == 0xE0)
		{

			strChar = sentence.substr(i, 3);
			i += 3;
			toword.text = strChar;
			toword.type = 1;
			wordData.push_back(toword);
			strChar = "";
		}
		else if ((chr & 0x80) == 0)
		{

			strChar = sentence.substr(i, 1);
			checkBlankEn = sentence.substr(i + 1, 1);
			++i;
			splitWordEn += strChar;
			if (!regex_match(checkBlankEn, patternEn) && !regex_match(checkBlankEn, patternBlank))
			{
				toword.text = splitWordEn;
				toword.type = 2;
				wordData.push_back(toword);
				splitWordEn = "";
			}
		}
	}

	for (int i = 0; i < wordData.size(); ++i)
	{
		_log("[CController] processTheText articleList vector2: %s", wordData.at(i).text.c_str());
	}
	return wordData;

}

vector<string> CController::phase(vector<toWord> &data)
{
	vector<string> output;
	string temp;
	string tempEn;
	for (int i = 0; i < data.size(); ++i)
	{
		if (data.at(i).type == 1)
		{
			temp += data.at(i).text.c_str();

			if ((i + 1) == data.size())
			{
				output.push_back(temp);
				temp = "";
				continue;
			}

			if (data.at(i + 1).type == 2)
			{
				output.push_back(temp);
				temp = "";
			}

		}
		else if (data.at(i).type == 2)
		{
			tempEn = data.at(i).text.c_str();
			output.push_back(tempEn);
			tempEn = "";
		}
	}

	for (int i = 0; i < output.size(); ++i)
	{
		_log("[CController] processTheText output: %s", output.at(i).c_str());
	}

	return output;
}

vector<string> CController::splitSentence(string &input)
{
	vector<string> wordData;
	string strChar = "";
	string strCharCh = "";
	string strCharChDouble = "";
	string blank = " ";
	string comma = ",";
	string reSentence = "";
	string checkSymbolEn = "";
	string checkSymbolCh = "";
	string checkBlankEn = "";
	string checkBlankCh = "";
	string splitWordEn = "";
	string splitWordCh = "";
	regex pattern("[A-Za-z0-9]");
	regex patternEn("[A-Za-z]");
	regex patternBlank("[ \f\n\r\t\v]");
	regex patternCh("\，|\。|\！|\：|\；|\“|\”|\（|\）|\、|\？|\《|\ 》|\「|\」|\～|\—|\﹏");
	regex patternPer("[\%]");
	string percent = "趴";
	CString temp;
	CString temp2;
	string check;

	//TODO: 依據空格斷句
	for (int i = 0; input[i] != '\0';)
	{
		char chr = input[i];
		//英文 chr是0xxx xxxx，即ascii碼
		if ((chr & 0x80) == 0)
		{
			strChar = input.substr(i, 1);
			checkBlankEn = input.substr(i + 1, 1);
			strCharCh = input.substr(i + 1, 3);
			strCharChDouble = input.substr(i + 1, 6);
			++i;
			if (!strChar.empty() && strChar != blank)
			{ //TODO: 判斷字詞是否空白
				if (checkBlankEn != blank)
				{
					splitWordEn += strChar;
					_log("[CController] 385 splitWordEn %s", splitWordEn.c_str());

					if (checkBlankEn == "\0" || !regex_match(checkBlankEn, pattern))
					{ // TODO: 判斷檢查格是否為字串末位或不匹配英文數字
						temp = strCharCh.c_str();
						temp2 = strCharChDouble.c_str();
						if ((temp.findOneOf(vWordUnit, check)) != -1 || (temp2.findOneOf(vWordUnitDouble, check)) != -1)
						{ //TODO: 數字單位是否匹配
							splitWordEn = num2Spell(splitWordEn);
							_log("[CController] 394 splitWordEn %s", splitWordEn.c_str());
							wordData.push_back(splitWordEn);
							splitWordEn = "";
						}
						else
						{
							if (regex_match(strChar, patternEn))
							{ //TODO: 判斷字詞是否為英文
								wordData.push_back(splitWordEn);
								splitWordEn = "";
							}
							else if (regex_match(strChar, patternPer))
							{
								splitWordEn = percent;
								wordData.push_back(splitWordEn);
								splitWordEn = "";
							}
							else
							{
								splitWordEn = num2Chinese(splitWordEn);
								_log("[CController] 414 splitWordEn %s", splitWordEn.c_str());
								wordData.push_back(splitWordEn);
								splitWordEn = "";
							}
						}
					}
				}
				else
				{
					if (regex_match(strChar, patternEn))
					{
						splitWordEn += strChar;
						_log("[CController] 426 splitWordEn %s", splitWordEn.c_str());
						wordData.push_back(splitWordEn);
						splitWordEn = "";
					}
					else if (regex_match(strChar, patternPer))
					{
						strChar = percent;
						splitWordEn += strChar;
						wordData.push_back(splitWordEn);
						splitWordEn = "";
					}
					else
					{
						splitWordEn += strChar;
						splitWordEn = num2Spell(splitWordEn);
						_log("[CController] 441 splitWordEn %s", splitWordEn.c_str());
						wordData.push_back(splitWordEn);
						splitWordEn = "";
					}
				}
			}
			else
			{
				strChar = blank;
//				continue;
				splitWordEn += strChar;
				_log("[CController] 452 splitWordEn %s", splitWordEn.c_str());
				wordData.push_back(splitWordEn);
				splitWordEn = "";
			}
		} //中文 chr是111x xxxx
		else if ((chr & 0xE0) == 0xE0)
		{
			strChar = input.substr(i, 3);
			checkBlankCh = input.substr(i + 3, 1);
			checkBlankEn = input.substr(i + 1, 1);
			i += 3;
			if (!strChar.empty() && strChar != blank)
			{
				if (checkBlankCh != blank)
				{
					splitWordCh += strChar;
					_log("[CController] 468 splitWordCh %s", splitWordCh.c_str());
					wordData.push_back(splitWordCh);
					splitWordCh = "";
					if (checkBlankCh == "\0" || regex_match(checkBlankCh, pattern))
					{
						wordData.push_back(splitWordCh);
						splitWordCh = "";
					}
				}
				else
				{
					splitWordCh += strChar;
					_log("[CController] 480 splitWordCh %s", splitWordCh.c_str());
					wordData.push_back(splitWordCh);
					splitWordCh = "";
				}
			}
		}
	}

	CString tempStore;
	CString tempBlank = " ";
	for (int i = 0; i < wordData.size(); ++i)
	{
		_log("[CController] processTheText wordData vector: %s", wordData.at(i).c_str());
	}

	return wordData;
}

bool CController::checkEnglish(string &input)
{
	for (int i = 0; input[i] != '\0';)
	{
		char chr;
		chr = input[i];
		if ((chr & 0x80) == 0)
		{
			string a = input.substr(i, 1);
			_log("[CController] is english :%s", a.c_str());
			return TRUE;
		}
		return FALSE;
	}
	return 0;
}

string CController::num2Chinese(string &num)
{
	size_t sz = num.size();
	string ret;
	int val;
	if (sz > MAX_LEN)
	{
		throw string("Exceeded max length");
	}
	if ("0" == num)
	{
		return CHAR_ZERO;
	}

	for (size_t i = 0; i != sz; ++i)
	{
		val = num[i] - '0';
		ret += CHAR_NUM[val];
	}
	return ret;
}

string CController::num2Spell(string &num)
{
	size_t sz = num.size(), revi, si, bi;
	bool lastZero = false, lastNonZero = false;
	int val;
	string ret;

	// limit the size
	if (sz > MAX_LEN)
	{
		throw string("Exceeded max length");
	}

	// special numbers
	if ("0" == num)
	{
		return CHAR_ZERO;
	}

	for (size_t i = 0; i != sz; ++i)
	{
		revi = (sz - 1) - i;
		bi = revi / INTERVAL;
		si = revi % INTERVAL;

		val = num[i] - '0';

		_log("[CController] val :%d", val);

		if (0 != val)
		{
			if (lastZero)
			{
				// append a zero
				ret += CHAR_ZERO;
			}

			// append a numeral
			if (1 == val && 1 == si)
			{
				// ten
				if (0 != i)
				{
					ret += CHAR_NUM[1];
				}
			}
			else
			{
				ret += CHAR_NUM[val];
			}

			if (0 != si)
			{
				// append a small interval char
				ret += CHAR_SI[si - 1];
			}

			lastNonZero = true;
			lastZero = false;
		}
		else
		{
			lastZero = true;
		}

		if (0 == si && 0 != bi)
		{
			if (lastNonZero)
			{
				// append a big interval char
				ret += CHAR_BI[bi - 1];
				// uncomment this to hide zero at rear of the big interval
				// followed up by another one
				//lastZero = false;
			}

			lastNonZero = false;
		}
		_log("[CController] ret1 :%s", ret.c_str());

	}
	return ret;
}

string CController::num2English(int val)
{
	string s = "";
	unsigned int v = val % 100;
	if (v <= 19)
	{
		if (v == 0)
		{
			s = "";
		}
		else
		{
			s = DictA[v];
		}
	}
	else if (v > 19 && v % 10 == 0)
	{
		s = DictB[v / 10];

	}
	else
	{
		s = ((v / 10) ? DictB[v / 10] + "-" : "") + DictA[v % 10];

	}
	if (val < 1000 && val % 100 == 0)
	{
		if (val / 100 > 0)
		{
			s = DictA[val / 100] + " Hundred" + s;
		}
	}
	else
	{
		s = ((val / 100) ? DictA[val / 100] + " Hundred and " : "") + s;
	}
	return s;
}

string CController::convert(string &num)
{
	int val = atoi(num.c_str());
	string result = "";
	int i = 0;
	do
	{
		if (i != 0)
			result = DictC[i] + " " + result;
		result = num2English(val % 1000) + " " + result;
		val = val / 1000;

		++i;
	}
	while (val != 0);
	return result;
}

//--------------------------------------------------//

void CController::removeFile(char *path)
{
	struct dirent *entry = NULL;
	DIR *dir = NULL;
	dir = opendir(path);
	while (entry = readdir(dir))
	{
		FILE *file = NULL;
		char* abs_path = new char[256];
		if (*(entry->d_name) != '.')
		{
			sprintf(abs_path, "%s/%s", path, entry->d_name);
			if (file = fopen(abs_path, "r"))
			{
				fclose(file);
				remove(abs_path);
			}
		}
		delete[] abs_path;
	}
}

/*
 * "user_id":"",
 "voice_id":0,
 "emotion":0,
 "text":"多型態角色語音智慧平台"
 */

void CController::onTTS(const int nSocketFD, const int nSequence, const char *szData)
{
	JSONObject jsonReq;
	JSONObject jsonResp;
	TTS_REQ ttsReq;
	CString strWave;
	CString strZip;
	CString strData;
	CString tempLabPath;
	string strResponseWav;
	string strResponseLabel;
	string strResponseData;
	jsonReq.load(szData);
	ttsReq.user_id = jsonReq.getString("user_id");
	ttsReq.voice_id = jsonReq.getInt("voice_id");
	ttsReq.emotion = jsonReq.getInt("emotion");
	ttsReq.text = jsonReq.getString("text");
	ttsReq.fm = jsonReq.getString("fm");
	ttsReq.g = jsonReq.getString("g");
	ttsReq.r = jsonReq.getString("r");
	ttsReq.id = jsonReq.getString("id");
	ttsReq.total = jsonReq.getInt("total");
	ttsReq.sequence_num = jsonReq.getInt("sequence_num");
	ttsReq.req_type = jsonReq.getInt("req_type");

	CString wavPath;
	CString outputPath;

	CString outputDir;

	time_t rawtime;
	time(&rawtime);

//	ttsReq.user_id = "";                    //--- kris for test ---//
//	ttsReq.voice_id = 103;
//	ttsReq.emotion = 2;
//	ttsReq.text = "多型態角色語音智慧平台";
//	ttsReq.fm = "";
//	ttsReq.b = "";
//	ttsReq.r = "";                         //---------------------//

	jsonReq.release();

	_log(
			"[CController] onTTS socketFD: %d text: %s user: %s voice: %d emotion: %d fm: %s g: %s r: %s id: %s total: %d sequence_sum: %d req_type: %d",
			nSocketFD, ttsReq.text.c_str(), ttsReq.user_id.c_str(), ttsReq.voice_id, ttsReq.emotion, ttsReq.fm.c_str(),
			ttsReq.g.c_str(), ttsReq.r.c_str(), ttsReq.id.c_str(), ttsReq.total, ttsReq.sequence_num, ttsReq.req_type);

	removeFile(WAV_PATH);

	jsonResp.create();

	outputDir.format("%s%ld", WAV_PATH, rawtime);
	mkdir(outputDir, 0777);

	if (ttsReq.req_type == 1)
	{
		textProcess->loadWordfromHTTP(ttsReq.text.c_str());
		jsonResp.put("status", 0);
		_log("[CController] change WordData success! %s:", ttsReq.text.c_str());
	}
	else if (ttsReq.req_type == 2)
	{
		tempLabPath.format("%s%s", DATA_PATH, ttsReq.text.c_str());
		remove(tempLabPath.getBuffer());
		jsonResp.put("status", 0);
	}
	else if (ttsReq.req_type == 3)
	{
		time_t currentTime = time(0);
		long int timeInteger = (long int) currentTime;
		struct tm *timeFormat = localtime(&timeInteger);
		char tempTime[100];
		strftime(tempTime, sizeof(tempTime), "%Y%m%d", timeFormat);
		string strCurrentTime = tempTime;
		strCurrentTime = strCurrentTime.assign(strCurrentTime, 0, 8);
		jsonResp.put("status", 0);
		jsonResp.put("data", strCurrentTime.c_str());
		_log("[CController] onTTS processTheText return currentTime: %s", strCurrentTime.c_str());
	}
	else
	{

//----------------- TODO: 如要genlabel 註解此處 ------------------//
		vector<string> splitData = splitSentence(ttsReq.text);
		ttsReq.text = "";

		for (vector<string>::iterator i = splitData.begin(); i != splitData.end(); ++i)
		{
			ttsReq.text = ttsReq.text + *i;
		}
		_log("[CController] ttsReq.text 22: %s", ttsReq.text.c_str());

		vector<string> splitData3 = parseSentence(ttsReq.text);
		ttsReq.text = "";

		for (vector<string>::iterator i = splitData3.begin(); i != splitData3.end(); ++i)
		{
			ttsReq.text = ttsReq.text + *i;
		}
		_log("[CController] ttsReq.text testtt 33: %s", ttsReq.text.c_str());
//-------------------------------------------------------------//

//----------------- TODO: 如要genlabel 註解此處 -----------------//

		for (vector<string>::iterator i = splitData3.begin(); i != splitData3.end(); ++i)
		{
			ttsReq.text = *i;

//			//----------- TODO: 檢查是否為英文  ----------//
			if (!checkEnglish(ttsReq.text))
			{
				_log("[CTextProcess] strFinded is Chinese: %s", ttsReq.text.c_str());
				//-------------------------------------------------//

//-------------------------------------------------------------//

				if (-1 == textProcess->processTheText(ttsReq, strWave, strZip, strData, count, outputDir))
				{
					jsonResp.put("status", 3);
				}
				else
				{
					if (ttsReq.voice_id == -1 && ttsReq.sequence_num == ttsReq.total)
					{  //TODO: voice id:-1 壓縮上傳Label
						_log("[CController] onTTS processTheText return zip: %s", strZip.getBuffer());
						strResponseLabel = strZip.toString().replace(0, strlen("/data/opt/tomcat/webapps"), TTS_HOST);
						jsonResp.put("status", 0);
						jsonResp.put("label", strResponseLabel.c_str());
					}
					else if (ttsReq.voice_id == -2)
					{  //TODO: voice id:-2 注音用Label
						_log("[CController] onTTS processTheText return data: %s", strData.getBuffer());
						strResponseData = strData.toString().replace(0, strlen("/data/opt/tomcat/webapps"), TTS_HOST);
						jsonResp.put("status", 0);
						jsonResp.put("data", strResponseData.c_str());
					}
					else
					{ //TODO: 中文合音
						_log("[CController] onTTS processTheText return wav: %s", strWave.getBuffer());
						strResponseWav = strWave.toString().replace(0, strlen("/data/opt/tomcat/webapps"), TTS_HOST);
//						jsonResp.put("status", 0);
//						jsonResp.put("wave", strResponseWav.c_str());
					}
				}
			}

//----------------- TODO: 如要genlabel 註解此處 -----------------//

			else
			{
				_log("[CTextProcess] strFinded is english: %s", ttsReq.text.c_str());
				if (-1 == textProcess->processTheText_EN(ttsReq, strWave, strZip, strData, count, outputDir))
				{
					jsonResp.put("status", 3);
				}
				else
				{	//TODO: 英文合音
					_log("[CController] onTTS processTheText_en return wav: %s", strWave.getBuffer());
					strResponseWav = strWave.toString().replace(0, strlen("/data/opt/tomcat/webapps"), TTS_HOST);
//					jsonResp.put("status", 0);
//					jsonResp.put("waveEN", strResponseWav.c_str());
				}
			}

//			//-------------------------------------------------//

			count++;

//			//----------- TODO: 從vector提取string 先暫時註解(550-552行相同)----------//
		}
		count = 1;
//			//-------------------------------------------------//

		wavPath.format("%s/*.wav", outputDir.getBuffer());
		outputPath.format("%s/%ld_0.wav", outputDir.getBuffer(), rawtime);
		char *wavPathChar = wavPath.getBuffer();
		char cmd[] = "sox";
		char *end = outputPath.getBuffer();
		char *args[4];
		args[0] = cmd;
		args[1] = wavPathChar;
		args[2] = end;
		args[3] = NULL;
		pid_t pid;
		int status;
		status = posix_spawn(&pid, "/usr/bin/sox", NULL, NULL, args, environ);
		if (status == 0)
		{
			_log("sox: Child pid: %d\n", pid);
			if (waitpid(pid, &status, 0) != -1)
			{
				_log("sox: Child exited with status %i\n", status);
			}
			else
			{
				_log("Error: waitpid");
			}
		}
		else
		{
			_log("sox: posix_spawn: %s\n", strerror(status));
		}
		outputPath = outputPath.toString().replace(0, strlen("/data/opt/tomcat/webapps"), TTS_HOST);
		jsonResp.put("status", 0);
		jsonResp.put("wave", outputPath.getBuffer());
	}

//-------------------------------------------------------------//

	cmpTTS->response(nSocketFD, tts_request, STATUS_ROK, nSequence, jsonResp.toJSON().c_str());
	jsonResp.release();

}

void CController::onHandleMessage(Message &message)
{
	switch (message.what)
	{
	case tts_request:
		thread([=]
		{	onTTS( message.arg[0], message.arg[1],message.strData.c_str());}).detach();
		break;
	}
}
