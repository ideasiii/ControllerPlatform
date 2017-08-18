/*
 * CController.cpp
 *
 *  Created on: 2017年8月18日
 *      Author: Jugo
 */

#include "CController.h"
#include "common.h"

CController::CController() :
		mnMsqKey(0)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{

	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	return TRUE;
}

int CController::onFinish(void* nMsqKey)
{
	return FALSE;
}

