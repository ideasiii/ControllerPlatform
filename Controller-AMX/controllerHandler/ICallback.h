/*
 * ICallback.h
 *
 *  Created on: 2016年11月1日
 *      Author: Jugo
 */

#pragma once

#define CB_AMX_COMMAND_CONTROL			1
#define CB_AMX_COMMAND_STATUS			2

typedef void (*CBFun)(void* param);

extern int sendCommand(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp, CSocket *socket);
extern int cmpSend(const int nSocket, const int nCommandId, const int nSequence, const char * szData);
