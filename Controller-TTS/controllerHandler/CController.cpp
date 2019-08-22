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

#include <fstream>
#include <dirent.h>
#define TTS_HOST				"http://175.98.119.122"
#define GEN_PATH            "/data/opt/tomcat/webapps/genlabel"     //kris 2019/04/11 read directory file
#define DATA_PATH			"/data/opt/tomcat/webapps/data/"

#define WAV_PATH            "/data/opt/tomcat/webapps/tts"

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
////-- standford auto connect
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
//	printf("\nfile %s\n", filepath.getBuffer());
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
//		_log("CController]fileName: %s", filename.getBuffer());
//		std::string FinalTitle = filename.getBuffer();
//		std::string FinalTitle2 = FinalTitle;
//		string wav = ".wav";
//		FinalTitle = FinalTitle.replace(FinalTitle.find(wav), sizeof(wav), "");
//		csTargetFileName.format("%s/txt/%s.txt",GEN_PATH,FinalTitle.c_str());
//		_log("CController]csTargetFileName: %s\n", csTargetFileName.getBuffer());
//		ifstream test(csTargetFileName, std::ifstream::in);
//		_log("[CController]131");
//		getline(test,datain);
//		_log("CController]dataIn : %s", datain.c_str());
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

vector<string> CController::splitSentence(string &input){
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
	regex pattern("[A-Za-z0-9]");
	regex patternCh("\，|\。|\！|\：|\；|\“|\”|\（|\）|\、|\？|\《|\ 》|\「|\」|\～|\—|\﹏");

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
	_log("[CController] processTheText reSentence: %s", reSentence.c_str());

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
		_log("[CController] processTheText wordData vector: %s", wordData.at(i).c_str());
	}
	return wordData;
}

bool CController::checkEnglish(string &input){
	for(int i; i < input.size(); i++){
		char chr;
		chr = input[i];
		if((chr & 0x80) == 0){
			return TRUE;
		}
	}
	return 0;
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

	string strFinded;

	strFinded = ttsReq.text.c_str();

//	ttsReq.user_id = "";                    //--- kris for test ---//
//	ttsReq.voice_id = 103;
//	ttsReq.emotion = 2;
//	ttsReq.text = "多型態角色語音智慧平台";
//	ttsReq.fm = "";
//	ttsReq.b = "";
//	ttsReq.r = "";                         //---------------------//

	jsonReq.release();

	_log("[CController] onTTS socketFD: %d text: %s user: %s voice: %d emotion: %d fm: %s g: %s r: %s id: %s total: %d sequence_sum: %d req_type: %d", nSocketFD, ttsReq.text.c_str(),
			ttsReq.user_id.c_str(), ttsReq.voice_id, ttsReq.emotion, ttsReq.fm.c_str(), ttsReq.g.c_str(), ttsReq.r.c_str(), ttsReq.id.c_str(), ttsReq.total, ttsReq.sequence_num, ttsReq.req_type);


//	//----------- TODO: 中英分詞 先暫時註解 ----------//
//	vector<string> splitData = splitSentence(ttsReq.text);
//	//---------------------//

	jsonResp.create();
	if (ttsReq.req_type == 1){
		textProcess->loadWordfromHTTP(ttsReq.text.c_str());
		jsonResp.put("status", 0);
		_log("[CController] change WordData success! %s:",  ttsReq.text.c_str());
	} else if(ttsReq.req_type == 2) {
		tempLabPath.format("%s%s", DATA_PATH, ttsReq.text.c_str());
		remove(tempLabPath.getBuffer());
		jsonResp.put("status", 0);
	} else if(ttsReq.req_type == 3) {
	    time_t currentTime = time(0);
		long int timeInteger = (long int)currentTime;
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

//			//----------- TODO: 從vector提取string 先暫時註解 ----------//
//		for (vector<string>::iterator i = splitData.begin(); i != splitData.end(); ++i)
//		{
//			strFinded = *i;
//			//-------------------------------------------------//


//			//----------- TODO: 檢查是否為英文 先暫時註解 ----------//
////			if (checkEnglish(strFinded) != 1)
////			{
////				_log("[CTextProcess] strFinded is chinese: %s", strFinded.c_str());
//			//-------------------------------------------------//


				if (-1 == textProcess->processTheText(ttsReq, strWave, strZip, strData, strFinded))
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
						jsonResp.put("status", 0);
						jsonResp.put("wave", strResponseWav.c_str());
					}
				}

//			//----------- TODO: 檢查是否為英文 先暫時註解 (344行-348行相同)----------//
//			}
//			else
//			{
//				_log("[CTextProcess] strFinded is english: %s", strFinded.c_str());
//				if (-1 == textProcess->processTheText_EN(ttsReq, strWave, strZip, strData, strFinded))
//				{
//					jsonResp.put("status", 3);
//				}
//				else
//				{	//TODO: 英文合音
//					_log("[CController] onTTS processTheText_en return wav: %s", strWave.getBuffer());
//					strResponseWav = strWave.toString().replace(0, strlen("/data/opt/tomcat/webapps"), TTS_HOST);
//					jsonResp.put("status", 0);
//					jsonResp.put("waveEN", strResponseWav.c_str());
//				}
//			}
//			//-------------------------------------------------//


//			//----------- TODO: 從vector提取string 先暫時註解(337-341行相同)----------//
//		}
//			//-------------------------------------------------//

	}
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
