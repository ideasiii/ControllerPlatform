/*
 * CWordParser.cpp
 *
 *  Created on: 2016年10月6日
 *      Author: Jugo
 */

#include "CWordParser.h"
#include "service.h"
#include "JSONObject.h"
#include "LogHandler.h"
#include "CDelivery.h"

using namespace std;

static CDelivery *delivery = new CDelivery;

CWordParser::CWordParser()
{

}

CWordParser::~CWordParser()
{

}

int CWordParser::parser(string strData, JSONObject &jsonOutput)
{

	if (strData.empty())
	{
		return ERR_INVALID_PARAMETER;
	}

	JSONObject jsonInput(strData);
	if (!jsonInput.isValid() || !jsonOutput.isValid())
		return ERR_INVALID_PARAMETER;

	_log("[Word] Input word: %s", strData.c_str());

	if (jsonInput.isNull("type") || jsonInput.isNull("words"))
		return ERR_INVALID_PARAMETER;

	int nType = jsonInput.getInt("type");
	int nLocal = jsonInput.getInt("local");
	string strWords = jsonInput.getString("words");
	JSONObject jsonoutput;
	JSONObject *pjson = &jsonoutput;
	int nResult = delivery->deliver(nType, nLocal, strWords.c_str(), &pjson);

	return 0;
}

