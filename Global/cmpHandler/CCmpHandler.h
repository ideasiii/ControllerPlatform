/*
 * CCmpHandler.h
 *
 *  Created on: 2015年10月21日
 *      Author: Louis Ju
 */

#pragma once
#include <string>
#include <vector>
#include "packet.h"
#include "LogHandler.h"
#include "CSocket.h"

template<typename T>
class CDataHandler;

using namespace std;

class CCmpHandler
{
public:
	static CCmpHandler* getInstance();
	virtual ~CCmpHandler();
	int getCommand(const void *pData);
	int getLength(const void *pData);
	int getStatus(const void *pData);
	int getSequence(const void *pData);
	void formatHeader(int nCommand, int nStatus, int nSequence, void ** pHeader);
	void formatRespPacket(int nCommand, int nStatus, int nSequence, void ** pHeader);
	void formatReqPacket(int nCommand, int nStatus, int nSequence, void ** pHeader);
	int formatPacket(int nCommand, void ** pPacket, int nBodyLen);
	void getSourcePath(const void *pData, char **pPath);
	int parseBody(int nCommand, const void *pData, CDataHandler<std::string> &rData);
	bool isAckPacket(int nCommand);
	int parseBody(const void *pData, vector<string> &vData);

private:
	CCmpHandler();

};

__attribute__ ((unused)) static int sendPacket(CSocket *socket, const int nSocket, const int nCommandId,
		const int nStatus, const int nSequence, const char * szData, bool isBigData = false)
{
	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	char *pIndex;

	memset(&packet, 0, sizeof(CMP_PACKET));

	packet.cmpHeader.command_id = htonl(nCommandId);
	packet.cmpHeader.command_status = htonl(nStatus);
	packet.cmpHeader.sequence_number = htonl(nSequence);

	if (isBigData == true)
	{
		char *reallyBigBuffer = new char[sizeof(CMP_HEADER) + strlen(szData) + 2];

		/* codes below are copy paste */
		nTotal_len = sizeof(CMP_HEADER);

		if (0 != szData)
		{
			memcpy(reallyBigBuffer + sizeof(CMP_HEADER), szData, strlen(szData));
			*(reallyBigBuffer + sizeof(CMP_HEADER) + strlen(szData)) = '\0';
			nTotal_len += strlen(szData) + 1;
		}

		packet.cmpHeader.command_length = htonl(nTotal_len);
		memcpy(reallyBigBuffer, (void*) &packet.cmpHeader, sizeof(CMP_HEADER));

		nRet = socket->socketSend(nSocket, reallyBigBuffer, nTotal_len);
		printPacket(nCommandId, STATUS_ROK, nSequence, nRet, "[Packet] Send", nSocket);

		string strLog;
		if (0 >= nRet)
		{
			_log("[Packet] CMP Send Fail socket: %d", nSocket);
		}

		delete[] reallyBigBuffer;
		return nRet;
	}
	else
	{
		pIndex = packet.cmpBody.cmpdata;

		/* codes below are copy paste */
		if (0 != szData)
		{
			memcpy(pIndex, szData, strlen(szData));
			pIndex += strlen(szData);
			nBody_len += strlen(szData);
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
		}

		nTotal_len = sizeof(CMP_HEADER) + nBody_len;
		packet.cmpHeader.command_length = htonl(nTotal_len);
		nRet = socket->socketSend(nSocket, &packet, nTotal_len);
		printPacket(nCommandId, STATUS_ROK, nSequence, nRet, "[Packet] Send", nSocket);

		string strLog;
		if (0 >= nRet)
		{
			_log("[Packet] CMP Send Fail socket: %d", nSocket);
		}

		return nRet;
	}

}

