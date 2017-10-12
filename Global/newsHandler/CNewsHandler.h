/*
 * CNewsHandler.h
 *
 *  Created on: 2017年9月25日
 *      Author: jugo
 */

#pragma once

#include <string>
#include <list>

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



typedef struct _NEWS
{
	std::string strTitle;
	std::string strDescription;
	std::string strLink;
} NEWS;

typedef struct _NEWS_DATE
{
	std::string strDate;
	std::list<NEWS> listNews;
} NEWS_DATE;

class CNewsHandler
{
public:
	explicit CNewsHandler();
	virtual ~CNewsHandler();
	int getNewsToday(NEWS_DATE &newDate);

private:
	xercesc::XercesDOMParser *mXmlParser;
};
