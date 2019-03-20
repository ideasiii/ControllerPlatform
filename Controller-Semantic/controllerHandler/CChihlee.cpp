/*
 * CChihlee.cpp
 *
 *  Created on: 2019年3月15日
 *      Author: jugo
 */

#include <string>
#include<fstream>
#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include "JSONObject.h"
#include "CChihlee.h"
#include "CString.h"
#include "LogHandler.h"
#include "CResponsePacket.h"
#include "CFileHandler.h"

using namespace std;

CChihlee::CChihlee()
{

}

CChihlee::~CChihlee()
{

}

void CChihlee::runAnalysis(const char *szInput, JSONObject &jsonResp)
{
	CFileHandler file;
	CString strWord = szInput;
	CResponsePacket respPacket;
	CString strText;
	CString strSound;
	CString strScreen;

	strWord.replace("笑訊", "校訓");
	strWord.replace("校去", "校訓");

	ofstream csWordFile("/chihlee/jetty/webapps/chihlee/Text.txt", ios::trunc);
	strText.format("%s\n          \n          ", szInput);
	csWordFile << strText.getBuffer() << endl;
	csWordFile.close();
	remove("/chihlee/jetty/webapps/chihlee/map.jpg");

	//=============== 校園導覽 =================================//
	if (0 <= strWord.find("導覽") || 0 <= strWord.find("地圖")|| 0 <= strWord.find("參觀")|| 0 <= strWord.find("校園"))
	{
		strScreen = "/chihlee/jetty/webapps/chihlee/wav/wav_1.wav";
		strSound = "/chihlee/jetty/webapps/chihlee/img/map.jpg";
	}

	//=============== 廁所怎麼走 =================================//
	if (0 <= strWord.find("廁所") || 0 <= strWord.find("洗手間") || 0 <= strWord.find("大便") || 0 <= strWord.find("小便")|| 0 <= strWord.find("方便間"))
	{
		strScreen = "/chihlee/jetty/webapps/chihlee/img/wc_map.jpg";
		strSound = "/chihlee/jetty/webapps/chihlee/wav/wav_2.wav";
	}

	//=============== 我想找電動輪椅充電 =================================//
	if (0 <= strWord.find("電動輪椅") || 0 <= strWord.find("輪椅充電") || 0 <= strWord.find("充電") || 0 <= strWord.find("沒電"))
	{
		strScreen = "/chihlee/jetty/webapps/chihlee/img/wc_map.jpg";
		strSound = "/chihlee/jetty/webapps/chihlee/wav/wav_3.wav";
	}

	//=============== 圖書館怎麼走 =================================//
	if (0 <= strWord.find("圖書館") || 0 <= strWord.find("圖館") || 0 <= strWord.find("書館") || 0 <= strWord.find("看書"))
	{
		strScreen = "/chihlee/jetty/webapps/chihlee/img/wc_map.jpg";
		strSound = "/chihlee/jetty/webapps/chihlee/wav/wav_4.wav";
	}

	//=============== 校訓 =================================//
	if (0 <= strWord.find("校訓")|| 0 <= strWord.find("誠信")|| 0 <= strWord.find("致理科大"))
	{
		strScreen = "/chihlee/jetty/webapps/chihlee/img/motto.png";
		strSound = "/chihlee/jetty/webapps/chihlee/wav/wav_5.wav";
	}

	//=============== 吉祥物 =================================//
	if (0 <= strWord.find("吉祥物")|| 0 <= strWord.find("喜鵲"))
	{
		strScreen = "/chihlee/jetty/webapps/chihlee/img/character.jpg";
		strSound = "/chihlee/jetty/webapps/chihlee/wav/wav_6.wav";
	}

	//=============== 校歌 =================================//
	if (0 <= strWord.find("校歌")|| 0 <= strWord.find("學生活動"))
	{
		strScreen = "/chihlee/jetty/webapps/chihlee/img/song.jpg";
		strSound = "/chihlee/jetty/webapps/chihlee/wav/wav_7.wav";
	}

	//================ 謝謝你的解說=====================//
	if (0 <= strWord.find("謝謝")|| 0 <= strWord.find("感謝")|| 0 <= strWord.find("掰掰"))
	{
		remove("/chihlee/jetty/webapps/chihlee/map.jpg");
		playSound("/chihlee/jetty/webapps/chihlee/wav/wav_8.wav");
	}

	file.copyFile(strScreen.getBuffer(), "/chihlee/jetty/webapps/chihlee/map.jpg");
	playSound(strSound.getBuffer());

	respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>("tts",
			"").format(jsonResp);
}

void CChihlee::playSound(const char *szWav)
{
	pid_t pid;
	int status = -1;

	if (szWav)
	{
		char *arg_list[] = { const_cast<char*>("aplay"), const_cast<char*>(szWav), NULL };

		status = posix_spawn(&pid, "/usr/bin/aplay", NULL, NULL, arg_list, environ);
		if (status == 0)
		{
			_log("[CController] importDB posix_spawn Child pid: %i", pid);
			if (waitpid(pid, &status, 0) != -1)
			{
				_log("[CController] importDB Child exited with status %i", status);
			}
			else
			{
				_log("[CController] importDB waitpid Error");
			}
		}
		else
		{
			_log("[CController] importDB Error posix_spawn: %s", strerror(status));
		}
	}
}

