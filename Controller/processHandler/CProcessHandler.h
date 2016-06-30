/*
 * CProcessHandler.h
 *
 *  Created on: 2016年6月27日
 *      Author: Jugo
 */

#pragma once

class CObject;

class CProcessHandler
{
public:
	explicit CProcessHandler();
	virtual ~CProcessHandler();
	int runProcess(void (*entry)(void));

private:

};
