#include "IReceiver.h"
#include "event.h"
#include "utility.h"
#include "common.h"
#include "CClientControllerMongoDB.h"
#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "JSONObject.h"
#include "packet.h"
#include "DynamicField.h"

static CClientControllerMongoDB * clientMongo = 0;

CClientControllerMongoDB::CClientControllerMongoDB() :
		CSocketClient(), cmpParser(CCmpHandler::getInstance()), dynamicField(new DynamicField)
{

	mapFunc[access_log_response] = &CClientControllerMongoDB::cmpAccessLogResponse;
	mapFunc[enquire_link_response] = &CClientControllerMongoDB::cmpEnquireLinkResponse;

}

CClientControllerMongoDB::~CClientControllerMongoDB()
{
	//stop();
}

CClientControllerMongoDB * CClientControllerMongoDB::getInstance()
{
	if (0 == clientMongo)
	{
		clientMongo = new CClientControllerMongoDB();
	}
	return clientMongo;
}

int CClientControllerMongoDB::startClient(string strIP, const int nPort, const int nMsqId)
{
	_DBG("[CClientControllerMongoDB] startClient");
	if (0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if (0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_MONGODB_RECEIVE);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_SERVER_DISCONNECT_MONGODB);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	setPacketConf(PK_CMP, PK_MSQ);

	const char* cszAddr = NULL;
	if (!strIP.empty())
	{
		cszAddr = strIP.c_str();
	}

	_TRY
		start(AF_INET, cszAddr, nPort);
	_CATCH

	if (!this->isValidSocketFD())
	{
		_log("[CClientControllerMongoDB] Socket Create Fail");
		return FALSE;
	}
	else
	{
		_log("[CClientControllerMongoDB] Connect Controller-MongoDB Success!!");
	}

	dynamicField->setMySQLInfo("175.98.119.121", "tracker", "ideas123!", "ideas", "field");

	return TRUE;
}

int CClientControllerMongoDB::sendCommand(void * param)
{
	CDataHandler<string> *strParam = reinterpret_cast<CDataHandler<string>*>(param);
	int nRet;
	if (dynamicField->isValidJSONFormat((*strParam)["data"]) == true)
	{
		dynamicField->insertDynamicData((*strParam)["data"]);
		dynamicField->printAllCaches();
		nRet = cmpAccessLogRequest((*strParam)["type"], (*strParam)["data"]);
	}
	else
	{
		_log("[CClientControllerMongoDB] is Not Valid JSON Format: %s", ((*strParam)["data"]).c_str());
	}

	return nRet;

}

void CClientControllerMongoDB::stopClient()
{
	stop();
}

void CClientControllerMongoDB::onReceive(const int nSocketFD, const void *pData)
{
	int nRet = -1;
	int nPacketLen = 0;
	CMP_HEADER cmpHeader;
	char *pPacket;

	pPacket = (char*) const_cast<void*>(pData);
	memset(&cmpHeader, 0, sizeof(CMP_HEADER));

	cmpHeader.command_id = cmpParser->getCommand(pPacket);
	cmpHeader.command_length = cmpParser->getLength(pPacket);
	cmpHeader.command_status = cmpParser->getStatus(pPacket);
	cmpHeader.sequence_number = cmpParser->getSequence(pPacket);

	printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
			"[CClientControllerMongoDB] Recv ", nSocketFD);

	if (cmpParser->isAckPacket(cmpHeader.command_id))
	{
		(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number,
				pPacket);
	}

}

int CClientControllerMongoDB::cmpAccessLogResponse(int nSocket, int nCommand, int nSequence, const void *pData)
{
	//deal with response
	return TRUE;
}

int CClientControllerMongoDB::cmpAccessLogRequest(string strType, string strLog)
{

	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	if (!clientMongo->isValidSocketFD())
	{
		return nRet;
	}

	CMP_PACKET packet;
	void *pHeader = &packet.cmpHeader;
	char *pIndex = packet.cmpBody.cmpdata;

	memset(&packet, 0, sizeof(CMP_PACKET));
	int accessLogSequence = getSequence();
	cmpParser->formatHeader( access_log_request, STATUS_ROK, accessLogSequence, &pHeader);

	int nType = -1;
	convertFromString(nType, strType);
	int net_type = htonl(nType);
	memcpy(pIndex, (const char*) &net_type, 4);
	pIndex += 4;
	nBody_len += 4;

	memcpy(pIndex, strLog.c_str(), strLog.length());
	pIndex += strLog.length();
	nBody_len += strLog.length();

	memcpy(pIndex, "\0", 1);
	++pIndex;
	++nBody_len;

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);

	_DBG("[controller] use Socket send AccessLog to MongoDB Controller");

	nRet = sendPacket(dynamic_cast<CSocket*>(clientMongo), clientMongo->getSocketfd(), access_log_request, STATUS_ROK,
			accessLogSequence, packet.cmpBody.cmpdata);

	printPacket( access_log_request, STATUS_ROK, accessLogSequence, nRet, "[CClientControllerMongoDB]",
			clientMongo->getSocketfd());

	return nRet;
}

int CClientControllerMongoDB::cmpEnquireLinkResponse(int nSocket, int nCommand, int nSequence, const void *pData)
{
	_log("[CServerMeeting] Get Enquire Link Response!");
	return 1;
}

