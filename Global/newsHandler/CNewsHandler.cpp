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

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

#define GOOGLE_NEWS			"https://news.google.com/news/rss/settings/sections?ned=tw&hl=zh-TW"
#define APPLE_NEWS			"http://www.appledaily.com.tw/rss/newcreate/kind/rnews/type/new"

using namespace xercesc;
using namespace std;

CNewsHandler::CNewsHandler()
{

}

CNewsHandler::~CNewsHandler()
{

}

int CNewsHandler::getNewsToday(NEWS_DATE &newDate)
{
	string strData;
	CHttpsClient httpsClient;
	set<string> setHead;

	httpsClient.GET(APPLE_NEWS, strData, setHead);

	_log("[CNewsHandler] getNewsToday .......................");

	return newDate.listNews.size();
}

