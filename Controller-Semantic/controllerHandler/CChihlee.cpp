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
#include "JSONObject.h"
#include "CChihlee.h"
#include "CString.h"
#include "LogHandler.h"

using namespace std;

CChihlee::CChihlee()
{

}

CChihlee::~CChihlee()
{

}

void CChihlee::runAnalysis(const char *szInput, JSONObject &jsonResp)
{
	CString strWord = szInput;

	_log("write chihlee text ==========================");
	ofstream csWordFile("/chihlee/jetty/webapps/chihlee/Text.txt", ios::trunc);
	csWordFile << szInput << endl;
	csWordFile.close();
	rename("/chihlee/jetty/webapps/chihlee/map.jpg", "/chihlee/jetty/webapps/chihlee/map_hide.jpg");
	_log("map rename    : map ----------> map hide");

	if (0 <= strWord.find("導覽") || 0 <= strWord.find("地圖"))
	{
		rename("/chihlee/jetty/webapps/chihlee/map_hide.jpg", "/chihlee/jetty/webapps/chihlee/map.jpg");
		_log("map rename    :  map hide ----------------> map");
	}
}

