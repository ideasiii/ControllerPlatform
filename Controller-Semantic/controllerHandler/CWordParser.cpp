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

using namespace std;

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

	if (jsonInput.isNull("type"))
		return ERR_INVALID_PARAMETER;

	int nType = jsonInput.getInt("type");

	switch(nType)
	{
	case
	}

	return ERR_SUCCESS;
}

