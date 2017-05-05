/*
 * CController.h
 *
 *  Created on: 2017年4月8日
 *      Author: Jugo
 */

#pragma once

#include "CApplication.h"

class CCmpWord;

class CController: public CApplication
{
public:
	CController();
	virtual ~CController();
	void onDeviceCommand(const CMPData * sendBackData);
	void onMeetingCommand(int nSocketFD, int nDataLen, const void *pData);
	void onMeetingCommand(const CMPData * mCMPData);
protected:
	int onCreated(void* nMsqKey);
	/**
	 *  Main Process run will callback onInitial
	 */
	int onInitial(void* szConfPath);

	/*
	 *  Main Process terminator will callback onFinish
	 */
	int onFinish(void* nMsqKey);
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
private:
	int startCmpWordServer(int nPort, int nMsqKey);

private:
	CCmpWord *cmpword;
	int mnMsqKey;
};
