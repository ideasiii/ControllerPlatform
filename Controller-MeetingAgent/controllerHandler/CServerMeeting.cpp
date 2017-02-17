#include "CSocketServer.h"
#include "event.h"
#include "packet.h"
#include "common.h"

#include "CServerMeeting.h"
#include "IReceiver.h"
#include "utility.h"

static CServerMeeting * serverMeeting = 0;

CServerMeeting::CServerMeeting() :
		CSocketServer()
{

}

CServerMeeting::~CServerMeeting()
{
	stopServer();
}

CServerMeeting * CServerMeeting::getInstance()
{
	if (0 == serverMeeting)
	{
		serverMeeting = new CServerMeeting();
	}
	return serverMeeting;
}

int CServerMeeting::startServer(string strIP, const int nPort, const int nMsqId)
{
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if (0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_MEETING_RECEIVE);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_MEETING);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_MEETING);
	}

	/** Set Receive , Packet is BYTE , Message Queue Handle **/
	setPacketConf(PK_BYTE, PK_MSQ);

	const char* cszAddr = NULL;
	if (!strIP.empty())
		cszAddr = strIP.c_str();
	if ( FAIL == start( AF_INET, cszAddr, nPort))
	{
		_log("[CServerMeeting]Socket Create Fail");
		return FALSE;
	}
	return TRUE;
}






int CServerMeeting::cmpBind(int nSocket, int nCommand, int nSequence, const void *pData)
{

	mapClient[nSocket] = nSocket;
	_log("[Server Device] Socket Client FD:%d Binded", nSocket);
	sendPacket(dynamic_cast<CSocket*>(serverMeeting), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, 0);
	return TRUE;
}

int CServerMeeting::cmpUnbind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	deleteClient(nSocket);
	sendPacket(dynamic_cast<CSocket*>(serverMeeting), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, 0);
	return TRUE;
}




void CServerMeeting::stopServer()
{
	stop();
}





bool CServerMeeting::onReceive(const int nSocketFD, const void *pData)
{

	return false;
}


void CServerMeeting::addClient(const int nSocketFD)
{
	_log("[CServerMeeting] Socket Client FD:%d Connected", nSocketFD);
}

void CServerMeeting::deleteClient(const int nSocketFD)
{
	if (mapClient.end() != mapClient.find(nSocketFD))
	{
		mapClient.erase(nSocketFD);
		_log("[CServerMeeting] Socket Client FD:%d UnBinded", nSocketFD);
	}
	_log("[CServerMeeting] Socket Client FD:%d Disconnected", nSocketFD);
}
