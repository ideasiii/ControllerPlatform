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

// Receive word
	int deliver(int, int, const void *, JSONObject **jsonOut);

private:
	typedef int (CDelivery::*MemFn)(int, int, const void *, JSONObject **jsonOut);
	MemFn fpTarget[SIZE];
};
