/*
 * ICallback.h
 *
 *  Created on: 2016年11月1日
 *      Author: Jugo
 */

#pragma once

#define CB_AMX_COMMAND			1

typedef void (*CBFun)(void* param);

extern int sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp, CSocket *socket);
