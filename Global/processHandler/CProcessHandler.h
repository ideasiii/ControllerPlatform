/*
 * CProcessHandler.h
 *
 *  Created on: 2016年6月27日
 *      Author: Jugo
 */

#pragma once

class CProcessHandler
{
public:
	explicit CProcessHandler();
	virtual ~CProcessHandler();
	static int runProcess(void (*entry)(int), int nMsqKey = -1);
	static int runProcess(void (*entry)());

private:

};

extern int process(void (*entry)(int), int nMsqKey);

