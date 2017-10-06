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
#include "CString.h"

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

#include <stdexcept>

#define GOOGLE_NEWS			"https://news.google.com/news/rss/settings/sections?ned=tw&hl=zh-TW"
#define APPLE_NEWS			"http://www.appledaily.com.tw/rss/newcreate/kind/rnews/type/new"

using namespace xercesc;
using namespace std;

CNewsHandler::CNewsHandler()
{
	try
	{
		XMLPlatformUtils::Initialize();  // Initialize Xerces infrastructure
	}
	catch(XMLException& e)
	{
		char* message = XMLString::transcode(e.getMessage());
		_log("[CNewsHandler] CNewsHandler XML toolkit initialization error :%s", XMLString::transcode(e.getMessage()));
		XMLString::release(&message);
	}

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

	if(!strData.empty())
	{
		_log("[CNewsHandler] getNewsToday :%s", strData.c_str());
		newDate.strDate = currentDate();

	}
	else
		_log("[CNewsHandler] getNewsToday get news Fail!!");

	return newDate.listNews.size();
}

