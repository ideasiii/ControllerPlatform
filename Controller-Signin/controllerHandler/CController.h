/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include "CObject.h"

class CSignin;
class CCmpSignin;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int startSignin(const char *szIP, const int nPort, const int nMsqId);
	int stop();

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
	void onTimer(int nId);

private:
	explicit CController();
	CSignin *signin;
	CCmpSignin *cmpSignin;

};
