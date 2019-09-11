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
#include "CMysqlHandler.h"
#include "common.h"

using namespace std;

CChihlee::CChihlee() :
		m_strMySQLIP("127.0.0.1")
{

}

CChihlee::~CChihlee()
{
	//delete mysql;
}

void CChihlee::init()
{
	CMysqlHandler *mysql = new CMysqlHandler();
	if (TRUE == mysql->connect(m_strMySQLIP.c_str(), "chihlee", "chihlee", "Chihlee123!", "5"))
	{
		_log("[CChihlee] init: Mysql Connect Success");
		listKeyWord.clear();
		mysql->query("select intent_id,type,word from keyWord order by wordLen desc", listKeyWord);

		string strField;
		string strValue;
		map<string, string> mapItem;

		for (list<map<string, string> >::iterator i = listKeyWord.begin(); i != listKeyWord.end(); ++i)
		{
			mapItem = *i;
			for (map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
			{
				printf("%s : %s\n", (*j).first.c_str(), (*j).second.c_str());
			}
		}

		// init fuzzy word table
		mapFuzzyWord.clear();
		list<map<string, string> > listFuzzy;
		mysql->query("select fuzzyWord,correctWord from fuzzyWord", listFuzzy);

		for (list<map<string, string> >::iterator i = listFuzzy.begin(); i != listFuzzy.end(); ++i)
		{
			mapItem = *i;
			mapFuzzyWord[mapItem["fuzzyWord"]] = mapItem["correctWord"];
			printf("%s : %s\n", mapItem["fuzzyWord"].c_str(), mapItem["correctWord"].c_str());
		}

		mysql->close();
		delete mysql;
	}
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
	strWord.replace("治理", "致理");
	strWord.trim();

	for (map<string, string>::iterator it = mapFuzzyWord.begin(); it != mapFuzzyWord.end(); ++it)
	{
		strWord.replace(it->first.c_str(), it->second.c_str());
	}

    //================ 謝謝你的解說=====================//
	if (0 <= strWord.find("謝謝") || 0 <= strWord.find("感謝") || 0 <= strWord.find("掰掰") || 0 <= strWord.find("拜拜")
			|| 0 <= strWord.find("謝啦") || 0 <= strWord.find("謝拉"))
	{
		remove("/opt/chihlee/jetty/webapps/chihlee/img/map.jpg");
		respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>("tts",
			"謝謝您的使用").format(jsonResp);
        return;
	}

	CMysqlHandler *mysql = new CMysqlHandler();
	if (TRUE == mysql->connect(m_strMySQLIP.c_str(), "chihlee", "chihlee", "Chihlee123!", "5")
			&& 0 < listKeyWord.size())
	{
		_log("[CChihlee] runAnalysis: Mysql Connect Success");

		int nIntent = -1;
		int nType = 0;
		string strField;
		string strValue;
		map<string, string> mapItem;
		int nCount = 0;
		// key word 字典檔查關鍵字
		for (list<map<string, string> >::iterator i = listKeyWord.begin(); i != listKeyWord.end(); ++i, ++nCount)
		{
			mapItem = *i;
			for (map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
			{
				printf("%s : %s\n", (*j).first.c_str(), (*j).second.c_str());
				if ((*j).first.compare("word") == 0)
				{
					if (0 <= strWord.find(j->second.c_str()))
					{
						printf("word march intent_id: %s type: %s word: %s\n", mapItem["intent_id"].c_str(),
								mapItem["type"].c_str(), (*j).second.c_str());
						nIntent = atoi(mapItem["intent_id"].c_str());
						nType = atoi(mapItem["type"].c_str());
						intent(nIntent, nType, (*j).second.c_str(), jsonResp, mysql);
						mysql->close();
						delete mysql;
						return;
					}
				}
			}
		}
		mysql->close();
		delete mysql;
	}

	remove("/opt/chihlee/jetty/webapps/chihlee/img/map.jpg");

	respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>("tts",
			"無法找到相關的資訊").format(jsonResp);
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
			_log("[CChihlee] playSound posix_spawn Child pid: %i", pid);
			if (waitpid(pid, &status, 0) != -1)
			{
				_log("[CChihlee] playSound Child exited with status %i", status);
			}
			else
			{
				_log("[CChihlee] playSound waitpid Error");
			}
		}
		else
		{
			_log("[CChihlee] playSound Error posix_spawn: %s", strerror(status));
		}
	}
}

/**
 * intent:
 * 1 : course
 * 2 : office
 */
void CChihlee::intent(int nIntent, int nType, const char* szWord, JSONObject &jsonResp, CMysqlHandler*& mysql)
{
	CResponsePacket respPacket;
	std::string strWord;

	// Intent search
	switch (nIntent)
	{
	case 1: // use course table
		strWord = course(nType, szWord, mysql);
		break;
	case 2: // use office table
		strWord = office(nType, szWord, mysql);
		break;
	default: // unknow intent
		strWord = "無法理解您的問題";
		break;
	}

	respPacket.setActivity<int>("type", RESP_TTS).setActivity<const char*>("lang", "zh").setActivity<const char*>("tts",
			strWord.c_str()).format(jsonResp);
}

/**
 * course:
 * 1 : 授課名稱
 * 2 : 授課老師
 * 3 : 授課地點
 */
string CChihlee::course(int nType, const char* szWord, CMysqlHandler* & mysql)
{
	std::string strResponse = "無法找到相關的課程資訊";
	_log("[CChihlee] course type: %d", nType);

	CString strSQL;
	std::string strDisplay;
	CFileHandler file;
	CString strScreen;
	CString picName;

	list<map<string, string> > listCourse;

	switch (nType)
	{
	case 1:
		strSQL.format("SELECT * FROM chihlee.course WHERE courseName like '%%%s%%';", szWord);
		break;
	case 2:
		strSQL.format("SELECT * FROM chihlee.course WHERE teacher like '%%%s%%';", szWord);
		break;
	case 3:
		strSQL.format("SELECT * FROM chihlee.classroom WHERE classroom like '%%%s%%';", szWord);
		break;
	}

	_log("[CChihlee] course SQL : %s", strSQL.getBuffer());
	mysql->query(strSQL.getBuffer(), listCourse);

	string strField;
	string strValue;
	map<string, string> mapItem;

	CString strTemplate;
	for (list<map<string, string> >::iterator i = listCourse.begin(); i != listCourse.end(); ++i)
	{
		if (i == listCourse.begin())
		{
			strResponse = "";
			strDisplay = "";
		}
		else
		{
			strDisplay += "\n";
		}
		mapItem = *i;

		if (3 == nType)
		{
			strTemplate.format("瞧這間教室的課表如圖");
			CString strDis;
			strDis.format("%s的課表",szWord);
			strDisplay = strDis.toString();
			for (list<map<string, string> >::iterator i = listCourse.begin(); i != listCourse.end(); ++i)
			{
				mapItem = *i;
				picName.format("/opt/chihlee/jetty/webapps/chihlee/img/pic_classroom/%s", mapItem["picName"].c_str());
				file.copyFile(picName.getBuffer(), "/opt/chihlee/jetty/webapps/chihlee/img/map.jpg");
				_log("[CChihlee] course classroom : %s", picName.getBuffer());
				break;
			}
		}
		else
		{
			strTemplate.format("%s在每週%s %s節,由%s老師在%s授課,,", mapItem["courseName"].c_str(), mapItem["weekDay"].c_str(),
					mapItem["credit"].c_str(), mapItem["teacher"].c_str(), mapItem["place"].c_str());
			strDisplay += mapItem["courseName"].c_str();
			strDisplay += "\n由";
			strDisplay += mapItem["teacher"].c_str();
			strDisplay += "授課";
		}

		strResponse += strTemplate.toString();

	}

	displayWord(strDisplay.c_str());

	if (2 == nType)
	{
		listCourse.clear();
		strSQL.format("SELECT * FROM chihlee.teacher WHERE teacher like '%%%s%%';", szWord);
		_log("[CChihlee] course SQL : %s", strSQL.getBuffer());
		mysql->query(strSQL.getBuffer(), listCourse);
		for (list<map<string, string> >::iterator i = listCourse.begin(); i != listCourse.end(); ++i)
		{
			mapItem = *i;
			picName.format("/opt/chihlee/jetty/webapps/chihlee/img/pic_teacher/%s", mapItem["picName"].c_str());
			file.copyFile(picName.getBuffer(), "/opt/chihlee/jetty/webapps/chihlee/img/map.jpg");
			_log("[CChihlee] course 老師課程表 : %s", picName.getBuffer());
		}

	}
	return strResponse;
}

string CChihlee::office(int nType, const char* szWord, CMysqlHandler*& mysql)
{
	string strField;
	string strValue;
	map<string, string> mapItem;
	CString strSQL;
	list<map<string, string> > listOffice;
	CString strTemplate;
	std::string strResponse = "無法找到相關的地點";
	_log("[CChihlee] office type: %d", nType);

	switch (nType)
	{
	case 1:
		strSQL.format("SELECT * FROM chihlee.office WHERE officeName like '%%%s%%';", szWord);
		break;
	}

	_log("[CChihlee] office SQL : %s", strSQL.getBuffer());
	mysql->query(strSQL.getBuffer(), listOffice);

	for (list<map<string, string> >::iterator i = listOffice.begin(); i != listOffice.end(); ++i)
	{
		if (i == listOffice.begin())
			strResponse = "";
		mapItem = *i;

		strTemplate.format("看, %s在%s%s樓,,", mapItem["officeName"].c_str(), mapItem["building"].c_str(),
				mapItem["floor"].c_str());
		strResponse += strTemplate.toString();
	}

	return strResponse;
}

void CChihlee::setMySQLIP(const char * szIP)
{
	if (0 == szIP)
		return;
	m_strMySQLIP = szIP;
}

void CChihlee::displayWord(const char * szWord)
{
	_log("[CChihlee] displayWord path: %s  word: %s", m_strWordPath.c_str(), szWord);
	CString strText;
	ofstream csWordFile(m_strWordPath.c_str(), ios::trunc);
	strText.format("%s\n          \n          ", szWord);
	csWordFile << strText.getBuffer() << endl;
	csWordFile.close();
}

void CChihlee::setWordPath(const char * szPath)
{
	if (0 == szPath)
		return;
	m_strWordPath = szPath;
}

void CChihlee::fuzzyWord(std::string &strWord)
{

}

