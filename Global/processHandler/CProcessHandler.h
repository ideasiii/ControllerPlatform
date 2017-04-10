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
	static int runProcess(void (*entry)(void));

private:

};

extern int process(void (*entry)(void));

