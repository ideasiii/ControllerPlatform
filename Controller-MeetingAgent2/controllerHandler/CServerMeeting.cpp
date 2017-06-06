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

CServerMeeting::CServerMeeting(CObject *object)
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
		nSocket = mapClient.back();
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

	_log("[CServerMeeting]SendCommand nRet %d", nRet);
	return nRet;
}

void CServerMeeting::onClientDisconnect(unsigned long int nSocketFD)
{
	_log("[CServerMeeting] onClientDisconnect %u", nSocketFD);
	deleteClient(nSocketFD);
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

	// 當 client 將要結束時，CCmpClient 內的 mapFunc 會被銷毀，但仍然可接收封包
	// 若這時送 unbind_response 給 client，會執行對方的 CCmpClient::onReceive()
	// CCmpClient::onReceive() 這時又去執行 mapFunc.find()
	// 將會導致 segmentation fault, 故不送 unbind response 給 client 
	//response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
	return TRUE;
}

void CServerMeeting::addClient(const int nSocketFD)
{
	//clean old client socket
	if(mapClient.size() > 0)
	{
		mapClient.clear();
	}
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


