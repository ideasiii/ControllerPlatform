/*
 * CNewsHandler.cpp
 *
 *  Created on: 2017年9月25日
 *      Author: root
 */

#include "CNewsHandler.h"
#include "CHttpsClient.h"
#include "utility.h"
#include "LogHandler.h"

#define GOOGLE_NEWS			"https://news.google.com/news/rss/settings/sections?ned=tw&hl=zh-TW"
#define APPLE_NEWS			"http://www.appledaily.com.tw/rss/newcreate/kind/rnews/type/new"

using namespace std;

CNewsHandler::CNewsHandler()
{

}

CNewsHandler::~CNewsHandler()
{

}

