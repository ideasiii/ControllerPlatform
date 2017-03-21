/*
 * CEvilTest.h
 *
 *  Created on: 2017年3月21日
 *      Author: Jugo
 */

#pragma once

class CThreadHandler;

class CEvilTest
{
public:
	CEvilTest(const char *szIP, int nPort);
	virtual ~CEvilTest();
	void start(int nCount);
	void run();

private:
	char mszIP[16];
	int mnPort;
	CThreadHandler *thread;
};
