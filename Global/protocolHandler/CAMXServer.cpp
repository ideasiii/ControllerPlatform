/*
 * CAMXServer.cpp
 *
 *  Created on: 2017年5月15日
 *      Author: Jugo
 */

#include <string>
#include "CAMXServer.h"
#include "AMXCommand.h"
#include "utility.h"

using namespace std;

CAMXServer::CAMXServer()
{

}

CAMXServer::~CAMXServer()
{

}

void CAMXServer::onTimer(int nId)
{

}

int CAMXServer::onTcpReceive(unsigned long int nSocketFD)
{
	int result;
	char pBuf[BUF_SIZE];
	void* pvBuf = pBuf;
	string strCommand;

	memset(pBuf, 0, sizeof(pBuf));
	result = socketrecv(nSocketFD, BUF_SIZE, &pvBuf);
	if (0 >= result)
		return 0;

	strCommand = pBuf;

	if (!strCommand.empty())
	{
		strCommand = trim(strCommand);
		if (0 == strCommand.substr(0, 4).compare("bind"))
		{
			bind(nSocketFD);
		}

		if (0 == strCommand.substr(0, 6).compare("unbind"))
		{
			unbind(nSocketFD);
		}

		if (0 != strCommand.substr(0, 6).compare(CTL_OK) && 0 != strCommand.substr(0, 9).compare(CTL_ERROR))
		{
			// Get Status Response
			if (AMX_STATUS_RESP.find(strCommand) != AMX_STATUS_RESP.end())
			{
				onAmxStatus(nSocketFD, strCommand.c_str());
			}

			// Update AMX_STATUS_CURRENT Hashmap
			if (AMX_STATUS_TO_CMD.find(strCommand) != AMX_STATUS_TO_CMD.end())
			{
				string strDevice = AMX_STATUS_TO_CMD[strCommand];
				AMX_STATUS_CURRENT[strDevice] = strCommand;
				_log("[CAMXServer] Update AMX %s Current Status: %s", strDevice.c_str(), strCommand.c_str());
			}
		}
	}
	else
	{
		response(nSocketFD, CTL_ERROR);
		_log("[CAMXServer] Error Receive AMX Command: %s From Socket: %d", strCommand.c_str(), nSocketFD);
	}

	return result;
}

int CAMXServer::request(const int nSocketFD, const char *szData)
{
	int nResult = FALSE;
	if (0 < nSocketFD)
	{
		nResult = sendPacket(nSocketFD, szData);
		_log("[CAMXServer] Send request Data:%s Length: %d Socket[%d]", szData, nResult, nSocketFD);
	}
	return nResult;
}

int CAMXServer::response(const int nSocketFD, const char *szData)
{
	int nResult = FALSE;
	if (0 < nSocketFD)
	{
		nResult = sendPacket(nSocketFD, szData);
		_log("[CAMXServer] Send response Data:%s Length: %d Socket[%d]", szData, nResult, nSocketFD);
	}
	return nResult;
}

int CAMXServer::sendPacket(const int nSocketFD, const char *szData)
{
	int nResult = FALSE;
	string strCommand = szData;

	if (0 < nSocketFD)
	{
		//nResult = socketSend(nSocketFD, strCommand.c_str(), strCommand.length());
		nResult = socketSend(nSocketFD, szData, strCommand.length());
		//_log("[CAMXServer] sendPacket, length:%d Data:%s", nResult, szData);
	}
	return nResult;
}

void CAMXServer::onClientConnect(unsigned long int nSocketFD)
{
	mapClient[nSocketFD] = nSocketFD;
	_log("[CAMXServer] onClientConnect Client FD:%d", nSocketFD);
}

void CAMXServer::onClientDisconnect(unsigned long int nSocketFD)
{
	mapClient.erase(nSocketFD);
	_log("[CAMXServer] onClientDisconnect Client FD:%d", nSocketFD);
}

void CAMXServer::bind(const int nSocketFD)
{
	mapClient[nSocketFD] = nSocketFD;
	_log("[CAMXServer] bind Client FD:%d", nSocketFD);
}
void CAMXServer::unbind(const int nSocketFD)
{
	mapClient.erase(nSocketFD);
	_log("[CAMXServer] unbind Client FD:%d", nSocketFD);
}
