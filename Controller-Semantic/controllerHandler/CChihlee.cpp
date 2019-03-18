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

	ofstream csWordFile("/chihlee/jetty/webapps/chihlee/Text.txt", ios::trunc);
	strText.format("%10 s\n          \n          ", szInput);
	csWordFile << strText.getBuffer() << endl;
	csWordFile.close();
	remove("/chihlee/jetty/webapps/chihlee/map.jpg");

	//=============== 校園導覽 =================================//
	if (0 <= strWord.find("導覽") || 0 <= strWord.find("地圖"))
	{
		file.copyFile("/chihlee/jetty/webapps/chihlee/img/map.jpg", "/chihlee/jetty/webapps/chihlee/map.jpg");
		//rename("/chihlee/jetty/webapps/chihlee/map_hide.jpg", "/chihlee/jetty/webapps/chihlee/map.jpg");
		playSound("/chihlee/jetty/webapps/chihlee/wav/wav_1.wav");
	}

	//=============== 廁所怎麼走 =================================//
	if (0 <= strWord.find("廁所") || 0 <= strWord.find("洗手間") || 0 <= strWord.find("大便") || 0 <= strWord.find("小便"))
	{
		file.copyFile("/chihlee/jetty/webapps/chihlee/img/wc_map.jpg", "/chihlee/jetty/webapps/chihlee/map.jpg");
		playSound("/chihlee/jetty/webapps/chihlee/wav/wav_2.wav");
	}

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

