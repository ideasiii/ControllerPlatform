/*
 * ICallback.h
 *
 *  Created on: 2016年11月1日
 *      Author: Jugo
 */

#pragma once

typedef void (*CBFun)(void* param);

extern int sendCommand(CSocket *socket, const int nSocket, const int nCommandId, const int nStatus, const int nSequence,
		const char * szData = 0);
