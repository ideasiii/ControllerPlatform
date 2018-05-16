/*
 * CPenReader.cpp
 *
 *  Created on: 2017年8月15日
 *      Author: Jugo
 */

#include <string>
#include "CPenReader.h"
#include "utility.h"
#include "CFileHandler.h"
#include "CResponsePacket.h"
#include "LogHandler.h"
#include "config.h"

using namespace std;

CPenReader::CPenReader()
{

}

CPenReader::~CPenReader()
{

}

int CPenReader::activity(const char *szInput, JSONObject& jsonResp)
{
	int nSerial;
	string strDisplay;
	string strFileName;
	string strTTS;
	CResponsePacket respPacket;
	string strContent;
	CFileHandler fh;

	_log("[CPenReader] activity read: %s", szInput);

	convertFromString(nSerial, szInput);

	switch(nSerial)
	{
	case 511:
		if(!fh.readContent("/data/opt/tomcat/webapps/story/三隻小豬.display", strContent))
			fh.readContent("/data/opt/tomcat/webapps/story/default.display", strContent);

		respPacket.setActivity("type", 1).setActivity("host", "https://ryejuice.sytes.net/story/").setActivity("file",
				"三隻小豬.mp3");
		if(!strContent.empty())
		{
			respPacket.setDisplay(strContent.c_str());
		}
		break;
	case 510:
		if(!fh.readContent("/data/opt/tomcat/webapps/story/小紅帽.display", strContent))
			fh.readContent("/data/opt/tomcat/webapps/story/default.display", strContent);
		respPacket.setActivity("type", 1).setActivity("host", "https://ryejuice.sytes.net/story/").setActivity("file",
				"小紅帽.mp3");
		if(!strContent.empty())
		{
			respPacket.setDisplay(strContent.c_str());
		}
		break;
	case 509:
		if(!fh.readContent("/data/opt/tomcat/webapps/story/小美人魚.display", strContent))
			fh.readContent("/data/opt/tomcat/webapps/story/default.display", strContent);
		respPacket.setActivity("type", 1).setActivity("host", "https://ryejuice.sytes.net/story/").setActivity("file",
				"小美人魚.mp3");
		if(!strContent.empty())
		{
			respPacket.setDisplay(strContent.c_str());
		}
		break;
	default:
		strDisplay =
				"{\"enable\":1,\"show\":[{\"time\":0,\"host\":\"https://ryejuice.sytes.net/edubot/mood/\",\"file\":\"emotion_sad.gif\",\"color\":\"#FFC2FF00\",\"description\":\"emotion_sad\",\"animation\":{\"type\":5,\"duration\":1000,\"repeat\":1,\"interpolate\":1},\"text\":{\"type\":0}}]}";
		respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>(
				"tts", format("點讀筆編碼 %d 目前無對應的服務", nSerial).c_str()).setDisplay(strDisplay.c_str());
		break;
	}

	respPacket.format(jsonResp);
	return 0;
}

