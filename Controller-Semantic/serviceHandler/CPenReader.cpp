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
	CResponsePacket respPacket;
	string strContent;
	CFileHandler fh;

	_log("[CPenReader] activity read: %s", szInput);

	convertFromString(nSerial, szInput);

	switch(nSerial)
	{
	case 511:
		fh.readContent("/data/opt/tomcat/webapps/story/三隻小豬.display", strContent);
		respPacket.setActivity("type", 1).setActivity("host", "https://smabuild.sytes.net/story/").setActivity("file",
				"三隻小豬.mp3");
		if(!strContent.empty())
		{
			respPacket.setDisplay(strContent.c_str());
		}
		respPacket.format(jsonResp);
		break;
	}

	return 0;
}

