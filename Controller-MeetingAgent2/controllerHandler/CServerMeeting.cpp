#include "CSocketServer.h"
#include "event.h"
#include "IReceiver.h"
#include "packet.h"
#include "common.h"
#include "ICallback.h"
#include "CDataHandler.cpp"
#include "CServerMeeting.h"
#include "utility.h"
#include <algorithm>

/** Enquire link function declare for enquire link thread **/
void *threadEnquireLinkRequest(void *argv);

CServerMeeting::CServerMeeting(CObject *object) :
		tdEnquireLink(new CThreadHandler)
{

	mpController = object;
}

CServerMeeting::~CServerMeeting()
{

}

int CServerMeeting::onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody)
{

	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, szBody);
	if (mCMPData.isVaild())
	{
		(*mapCallback[CB_DEVCIE_COMMAND])(static_cast<void*>(const_cast<CMPData*>(&mCMPData)));
	}
	else
	{
		return FALSE;
	}
	return TRUE;

}


int CServerMeeting::sendCommand(int commandID, int seqNum, string bodyData)
{
	if (bodyData.size() > 0)
	{
		_log("[CServerMeeting] send command %d, seqNum is %d, data: %s\n", commandID, seqNum, bodyData.c_str());
	}
	else
	{
		_log("[CServerMeeting] send command %d, seqNum is %d\n", commandID, seqNum);
	}
	vector<int>::iterator it;

	int nSocket = -1;

	if (mapClient.size() > 0)
	{
		nSocket = mapClient.front();
	}
	int nRet = 0;

	if (nSocket >= 0)
	{
		if (bodyData.size() > 0)
		{
			nRet = request(nSocket, commandID, STATUS_ROK, seqNum, bodyData.c_str());
		}
		else
		{
			nRet = request(nSocket, commandID, STATUS_ROK, seqNum, 0);
		}
	}
	else
	{
		_log("[CServerMeeting] ERROR to find Controller-Meeting Socket ID!");
	}

	if (nRet <= 0)
	{
		//mapClient.erase(mapClient.begin());
	}
	_log("[CServerMeeting]SendCommand nRet %d");
	return nRet;
}

int CServerMeeting::onBind(int nSocket, int nCommand, int nSequence, const void *pData)
{

	addClient(nSocket);

	_log("[CServerMeeting] Socket Controller-Meeting Client FD:%d Binded", nSocket);
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);

	return TRUE;
}

int CServerMeeting::onUnbind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	deleteClient(nSocket);

	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
	return TRUE;
}






void CServerMeeting::addClient(const int nSocketFD)
{
	mapClient.push_back(nSocketFD);
	_log("[CServerMeeting] Socket Client FD:%d Connected", nSocketFD);
}

void CServerMeeting::deleteClient(const int nSocketFD)
{
	vector<int>::iterator position = find(mapClient.begin(), mapClient.end(), nSocketFD);

	if (position != mapClient.end())
	{
		mapClient.erase(position);
		_log("[CServerMeeting] Socket Client FD:%d UnBinded", nSocketFD);
	}

	_log("[CServerMeeting] Socket Client FD:%d Disconnected", nSocketFD);
}

CMPData CServerMeeting::parseCMPData(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	if (szBody != NULL)
	{

		const char *pBody = reinterpret_cast<const char*>(szBody);
		return CMPData(nSocket, nCommand, nSequence, pBody);
	}
	else
	{
		return CMPData(nSocket, nCommand, nSequence, "");
	}

}
void CServerMeeting::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

void CServerMeeting::runEnquireLinkRequest()
{
	int nSocketFD = -1;
	string strSql;
	string strLog;

	while (1)
	{
		tdEnquireLink->threadSleep(10);

		for (size_t i = 0; i < mapClient.size(); i++)
		{
			nSocketFD = mapClient[i];
			int nRet = cmpEnquireLinkRequest(nSocketFD);

			if (nRet > 0)
			{
				//Enquire Link Success
			}
			else
			{
				//Enquire Link Failed
				_log("[CServerMeeting] Send Enquire Link Failed result = %d\n", nRet);
			}
		}

	}
}

int CServerMeeting::cmpEnquireLinkRequest(const int nSocketFD)
{
	return sendCommand(enquire_link_request, getSequence(), "");
}

/************************************* thread function **************************************/
void *threadEnquireLinkRequest(void *argv)
{
	CServerMeeting* ss = reinterpret_cast<CServerMeeting*>(argv);
	ss->runEnquireLinkRequest();
	return NULL;
}

