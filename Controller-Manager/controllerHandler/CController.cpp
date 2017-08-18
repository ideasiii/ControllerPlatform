/*
 * CController.cpp
 *
 *  Created on: 2017年8月18日
 *      Author: Jugo
 */

#include "CController.h"
#include "common.h"
#include "CManager.h"

CController::CController() :
		mnMsqKey(0), manager(0)
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
	manager = new CManager(this);
	return TRUE;
}

int CController::onFinish(void* nMsqKey)
{
	delete manager;
	return FALSE;
}

void CController::onHandleMessage(Message &message)
{
	switch(message.what)
	{

	}
}

