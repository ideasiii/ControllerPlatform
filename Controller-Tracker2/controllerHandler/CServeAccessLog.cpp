#include "IReceiver.h"
#include "event.h"
#include "common.h"

#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "CServerAccessLog.h"
#include "iCommand.h"
#include "packet.h"

CServerAccessLog::CServerAccessLog()
{

}

CServerAccessLog::~CServerAccessLog()
{
}

int CServerAccessLog::onSignin(int nSocket, int nCommand, int nSequence, const void *pData)
{
	return 0;
}

int CServerAccessLog::onAccesslog(int nSocket, int nCommand, int nSequence, const void *pData)
{

	_DBG("[CServerAccessLog]cmpAccessLogRequest");
	sendPacket(dynamic_cast<CSocket*>(this), nSocket, nCommand | generic_nack, STATUS_ROK, nSequence, 0);

	CDataHandler<string> rData;

	if (paseBody(pData, rData) > 0)
	{
		//mongoDB
		_log("[CServerAccessLog] call back mongo client@@");

		//char *tmp = new char[rData["data"].size() + 1];
		//strcpy(tmp, rData["data"].c_str());

		//void * data = rData["data"].c_str();

		//_log("data: %s", tmp);

		(*mapCallback[CB_CONTROLLER_MONGODB_COMMAND])((void*) rData["data"].c_str());

		//mysql

	}
	else
	{
		_log("[CServerAccessLog]Error while covert access log!");

	}

	return TRUE;

}

void CServerAccessLog::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

int CServerAccessLog::paseBody(const void *pData, CDataHandler<string> &rData)
{
	int nStrLen = 0;
	int nTotalLen = 0;
	int nBodyLen = 0;
	int nType = 0;
	char * pBody;
	char temp[MAX_SIZE];

	nTotalLen = getLength(pData);

	_DBG("[CServerAccessLog] sockect total Length: %d", nTotalLen);

	nBodyLen = nTotalLen - sizeof(CMP_HEADER);

	if (0 < nBodyLen)
	{
		pBody = (char*) ((char *) const_cast<void*>(pData) + sizeof(CMP_HEADER));
		nType = ntohl(*((int*) pBody));
		rData.setData("type", ConvertToString(nType));

		pBody += 4;
		if (isValidStr((const char*) pBody, MAX_SIZE))
		{
			memset(temp, 0, sizeof(temp));
			strcpy(temp, pBody);
			rData.setData("data", temp);
			nStrLen = strlen(temp);
			++nStrLen;
			pBody += nStrLen;
		}
		else
		{
			return -1;
		}
	}
	return 1;
}

int CServerAccessLog::getLength(const void *pData)
{
	int nLength;
	CMP_HEADER *pHeader;
	pHeader = (CMP_HEADER *) pData;
	nLength = ntohl(pHeader->command_length);
	return nLength;
}

