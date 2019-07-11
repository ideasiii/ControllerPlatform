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
//		printf("%s\n", entry->d_name);
//		printf("filename: %s\n", filename.getBuffer());
//		std::string FinalTitle = filename.getBuffer();
//		std::string FinalTitle2 = FinalTitle;
//		string wav = ".wav";
//		FinalTitle = FinalTitle.replace(FinalTitle.find(wav), sizeof(wav), "");
//		csTargetFileName.format("%s/txt/%s.txt",GEN_PATH,FinalTitle.c_str());
//		printf("csTargetFileName: %s\n", csTargetFileName.getBuffer());
//		ifstream test(csTargetFileName, std::ifstream::in);
//		getline(test,datain);
//		printf("datain : %s\n", datain.c_str());
//		test.close();
//		textProcess->strFileTitle_gen = FinalTitle2.c_str();
//		textProcess->strInput_gen = datain;
//		textProcess->genLabels();
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
	jsonResp.create();
	if (ttsReq.req_type == 1){
		textProcess->loadWordfromHTTP(ttsReq.text.c_str());
		jsonResp.put("status", 0);
		_log("[CController] change WordData success! %s:",  ttsReq.text.c_str());
	} else if(ttsReq.req_type == 2) {
		tempLabPath.format("%s%s", DATA_PATH, ttsReq.text.c_str());
		remove(tempLabPath.getBuffer());
		jsonResp.put("status", 0);
	} else {
		if (-1 == textProcess->processTheText(ttsReq, strWave, strZip, strData))  //kris call by reference
		{
			jsonResp.put("status", 3);
		}else{
			if (ttsReq.voice_id == -1 && ttsReq.sequence_num == ttsReq.total )  //kris call by reference
			{
				_log("[CController] onTTS processTheText return zip: %s", strZip.getBuffer());
				strResponseLabel = strZip.toString().replace(0, strlen("/data/opt/tomcat/webapps"), TTS_HOST);
				jsonResp.put("status", 0);
				jsonResp.put("label", strResponseLabel.c_str());
			} else if(ttsReq.voice_id == -2){
				_log("[CController] onTTS processTheText return data: %s", strData.getBuffer());
				strResponseData = strData.toString().replace(0, strlen("/data/opt/tomcat/webapps"), TTS_HOST);
				jsonResp.put("status", 0);
				jsonResp.put("data", strResponseData.c_str());
			} else {
				_log("[CController] onTTS processTheText return wav: %s", strWave.getBuffer());
				strResponseWav = strWave.toString().replace(0, strlen("/data/opt/tomcat/webapps"), TTS_HOST);
				jsonResp.put("status", 0);
				jsonResp.put("wave", strResponseWav.c_str());
			}
		}
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
