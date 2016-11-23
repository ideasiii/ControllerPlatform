/*
 * CDelivery.cpp
 *
 *  Created on: 2016年10月7日
 *      Author: Jugo
 */

#include "CDelivery.h"
#include "service.h"
#include "JSONObject.h"
#include "LogHandler.h"

CDelivery::CDelivery()
{
	fpTarget[TARGET_SEMANTIC] = &CDelivery::semantic;
	fpTarget[TARGET_COMMAND] = &CDelivery::command;
}

CDelivery::~CDelivery()
{

}

int CDelivery::deliver(int nType, int nLocal, const char * szWord, JSONObject **jsonOut)
{
	if (0 > nType || SIZE <= nType)
		return ERR_INVALID_PARAMETER;

	_log("[Delivery] Type: %d Local: %d Word: %s", nType, nLocal, szWord);
	return (this->*this->fpTarget[nType])(nType, nLocal, szWord, jsonOut);
}

int CDelivery::semantic(int nType, int nLocal, const char * szWord, JSONObject **jsonOut)
{
	int nResult = ERR_SUCCESS;

	return nResult;
}

int CDelivery::command(int nType, int nLocal, const char * szWord, JSONObject **jsonOut)
{
	int nResult = ERR_SUCCESS;

	return nResult;
}

