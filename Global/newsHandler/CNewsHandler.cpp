/*
 * CNewsHandler.cpp
 *
 *  Created on: 2017年9月25日
 *      Author: root
 */

#include <stdexcept>
#include "CNewsHandler.h"
#include "CHttpsClient.h"
#include "utility.h"
#include "LogHandler.h"
#include "CString.h"

#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#define GOOGLE_NEWS			"https://news.google.com/news/rss/settings/sections?ned=tw&hl=zh-TW"
#define APPLE_NEWS			"http://www.appledaily.com.tw/rss/newcreate/kind/rnews/type/new"

using namespace xercesc;
using namespace std;

CNewsHandler::CNewsHandler() :
		mXmlParser(0)
{
	try
	{
		XMLPlatformUtils::Initialize();  // Initialize Xerces infrastructure
		_log("[CNewsHandler] CNewsHandler Initialize Success");
	}
	catch(XMLException& e)
	{
		char* message = XMLString::transcode(e.getMessage());
		_log("[CNewsHandler] CNewsHandler XML toolkit initialization error :%s", XMLString::transcode(e.getMessage()));
		XMLString::release(&message);
	}

	mXmlParser = new XercesDOMParser;
	// Configure DOM parser.
	mXmlParser->setValidationScheme(XercesDOMParser::Val_Never);
	mXmlParser->setDoNamespaces(false);
	mXmlParser->setDoSchema(false);
	mXmlParser->setLoadExternalDTD(false);

}

CNewsHandler::~CNewsHandler()
{
	// Terminate Xerces
	delete mXmlParser;
	try
	{
		XMLPlatformUtils::Terminate();  // Terminate after release of memory
		_log("[CNewsHandler] ~CNewsHandler Terminate Success");
	}
	catch(xercesc::XMLException& e)
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());
		_log("[CNewsHandler] ~CNewsHandler XML toolkit Terminate error :%s", XMLString::transcode(e.getMessage()));
		XMLString::release(&message);
	}

}

int CNewsHandler::getNewsToday(NEWS_DATE &newDate)
{
	string strData;
	CHttpsClient httpsClient;
	set<string> setHead;
	DOMNode* currentNode;
	DOMElement* currentElement;
	DOMNodeList* channelChildren;
	DOMNodeList* itemChildren;

	httpsClient.GET(APPLE_NEWS, strData, setHead);

	if(!strData.empty())
	{
		//	_log("[CNewsHandler] getNewsToday :%s", strData.c_str());
		newDate.strDate = currentDate();

		MemBufInputSource src((const XMLByte*) strData.c_str(), strData.length(), "data", false);

		try
		{
			mXmlParser->parse(src);

			// no need to free this pointer - owned by the parent parser object
			DOMDocument* xmlDoc = mXmlParser->getDocument();

			if(!xmlDoc)
			{
				_log("[CNewsHandler] getNewsToday xmlDoc invalid");
				return -1;
			}
			// Get the top-level element: NAme is "root". No attributes for "root"
			DOMElement* elementRoot = xmlDoc->getDocumentElement();
			if(!elementRoot)
			{
				_log("[CNewsHandler] getNewsToday xml root invalid");
				return -1;
				//throw(std::runtime_error("empty XML document"));
			}

			DOMNodeList* children = elementRoot->getChildNodes();
			const XMLSize_t nodeCount = children->getLength();

			for(XMLSize_t xx = 0; xx < nodeCount; ++xx)
			{
				currentNode = children->item(xx);

				if(currentNode->getNodeType() &&  // true is not NULL
						currentNode->getNodeType() == DOMNode::ELEMENT_NODE) // is element
				{
					currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);
					//	_log(" *** %s", XMLString::transcode(currentElement->getTagName()));
					if(XMLString::equals(currentElement->getTagName(), XMLString::transcode("channel")))
					{
						channelChildren = currentElement->getChildNodes();
						const XMLSize_t channelNodeCount = channelChildren->getLength();
						for(XMLSize_t channelcd = 0; channelcd < channelNodeCount; ++channelcd)
						{
							currentNode = channelChildren->item(channelcd);
							if(currentNode->getNodeType() &&  // true is not NULL
									currentNode->getNodeType() == DOMNode::ELEMENT_NODE) // is element
							{
								currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);
								//	_log(" *** %s", XMLString::transcode(currentElement->getTagName()));
								if(XMLString::equals(currentElement->getTagName(), XMLString::transcode("item")))
								{
									itemChildren = currentElement->getChildNodes();
									const XMLSize_t itemNodeCount = itemChildren->getLength();
									for(XMLSize_t itemcd = 0; itemcd < itemNodeCount; ++itemcd)
									{
										currentNode = itemChildren->item(itemcd);
										if(currentNode->getNodeType() &&  // true is not NULL
												currentNode->getNodeType() == DOMNode::ELEMENT_NODE) // is element
										{
											currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);
											if(XMLString::equals(currentElement->getTagName(),
													XMLString::transcode("title")))
											{
												//currentElement->getNodeValue();
												_log("%s", XMLString::transcode(currentElement->getTextContent()));
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		catch(xercesc::XMLException& e)
		{
			char* message = xercesc::XMLString::transcode(e.getMessage());
			_log("[CNewsHandler] getNewsToday XML toolkit Terminate error :%s", XMLString::transcode(e.getMessage()));
			XMLString::release(&message);
		}
	}
	else
		_log("[CNewsHandler] getNewsToday get news Fail!!");

	return newDate.listNews.size();
}

