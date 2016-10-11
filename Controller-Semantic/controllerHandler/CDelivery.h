/*
 * CDelivery.h
 *
 *  Created on: 2016年10月7日
 *      Author: Jugo
 */

#pragma once

typedef enum _TARGET
{
	TARGET_SEMANTIC = 0, TARGET_COMMAND, SIZE
} TARGET;

class JSONObject;

class CDelivery
{
public:
	CDelivery();
	virtual ~CDelivery();

	int deliver(int nType, int nLocal, const char * szWord, JSONObject **jsonOut);

private:
	typedef int (CDelivery::*MemFn)(int, int, const char *, JSONObject **jsonOut);
	MemFn fpTarget[SIZE];
	int semantic(int nType, int nLocal, const char * szWord, JSONObject **jsonOut);
	int command(int nType, int nLocal, const char * szWord, JSONObject **jsonOut);
};
