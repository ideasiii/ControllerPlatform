/*
 * CCmpServer.cpp
 *
 *  Created on: 2017年3月16日
 *      Author: Jugo
 */

#include "CCmpServer.h"
#include "LogHandler.h"

CCmpServer::CCmpServer()
{

}

CCmpServer::~CCmpServer()
{

}

void CCmpServer::onTimer(int nId)
{

}

void CCmpServer::onReceive(unsigned long int nId, int nDataLen, const void* pData)
{
	_log("[CCmpServer] onReceive, Socket FD: %lu", nId);
}
