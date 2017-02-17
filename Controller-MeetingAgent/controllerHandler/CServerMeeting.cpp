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

void CServerMeeting::stopServer()
{
	stop();
}





bool CServerMeeting::onReceive(const int nSocketFD, const void *pData)
{

	return false;
}

