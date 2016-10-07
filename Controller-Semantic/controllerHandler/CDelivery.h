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

class CDelivery
{
public:
	CDelivery();
	virtual ~CDelivery();

private:
	typedef int (CDelivery::*MemFn)(int, int, const void *);
	MemFn fpTarget[SIZE];
};
