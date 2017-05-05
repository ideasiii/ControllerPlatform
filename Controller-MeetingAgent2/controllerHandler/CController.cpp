
#include <stdio.h>
#include "CController.h"
#include "common.h"
#include "CConfig.h"
#include "utility.h"
#include "event.h"
#include "CServerDevice.h"
#include "CServerMeeting.h"
#include "iCommand.h"
#include "JSONObject.h"
#include <string>

using namespace std;

CController::CController() :
		cmpword(0), mnMsqKey(-1)
{

}

CController::~CController()
{

}


/**
 * Define Socket Client ReceiveFunction
 */
int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
	//controlcenter->receiveCMP(nSocketFD, nDataLen, pData);
	return 0;
}

/**
 *  Define Socket Server Receive Function
 */
int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
{
	mtx.lock();
	controller->onMeetingCommand(nSocketFD, nDataLen, pData);
	mtx.unlock();
	return 0;
}

void IonMeetingCommand(void *param)
{
	const CMPData *strParam = reinterpret_cast<const CMPData*>(param);
	this->onMeetingCommand(strParam);
}

void IonDeviceCommand(void *param)
{
	const CMPData *strParam = reinterpret_cast<const CMPData*>(param);
	this->onDeviceCommand(strParam);
}
void CController::onMeetingCommand(int nSocketFD, int nDataLen, const void *pData)
{

	//const CMP_PACKET * cmpPacket = reinterpret_cast<const CMP_PACKET *>(pData);
	//_log("[CController] cmpPacket body: %s",cmpPacket->cmpBodyUnlimit.cmpdata);

	serverMeeting->controllerCallBack(nSocketFD, nDataLen, pData);
}
void CController::onDeviceCommand(const CMPData * sendBackData)
{
	map<int, CMPData>::iterator itr = deviceMapData.find(sendBackData->nSequence);
	if (itr == deviceMapData.end())
	{
		//not found
		_log("[CController] Cannot find this data which nSequence = %d\n", sendBackData->nSequence);
	}
	else
	{
		//found it!

		CMPData *deviceData = new CMPData(&(itr->second));

		deviceMapData.erase(itr->first);

		serverDevice->sendCommand(deviceData->nFD, sendBackData->nCommand, deviceData->nSequence,
				sendBackData->bodyData);
	}
}
void CController::onMeetingCommand(const CMPData * mCMPData)
{
	int controllerMeetingSeqNum = getSequence();

	deviceMapData[controllerMeetingSeqNum] = *mCMPData;
	serverMeeting->sendCommand(mCMPData->nCommand, controllerMeetingSeqNum, mCMPData->bodyData);
}
int CController::onCreated(void* nMsqKey)
{
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_MEETING_AGENT; //*(reinterpret_cast<int*>(nMsqKey));
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	string strConfPath = reinterpret_cast<const char*>(szConfPath);
	if (strConfPath.empty())
		return FALSE;

	int nPort;
	string strDevicePort;
	string strControllerMeetingPort;
	CConfig *config;
	config = new CConfig();
	if (config->loadConfig(strConfPath))
	{
		strDevicePort = config->getValue("SERVER DEVICE", "port");
		if (!strDevicePort.empty())
		{
			convertFromString(nPort, strDevicePort);
			startServerDevice(0,  nPort,-1);
		}

		strControllerMeetingPort = config->getValue("SERVER CONTROLLER_METTING", "port");
		if (!strControllerMeetingPort.empty())
		{
			convertFromString(nPort, strControllerMeetingPort);
			startServerMeeting(0,nPort, -1);
		}

	}
	delete config;
	return TRUE;
}

int CController::onFinish(void* nMsqKey)
{
	if (0 != cmpword)
	{
		delete cmpword;
		cmpword = 0;
	}
	return TRUE;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_TCP_MEETING_RECEIVE:
		_log("[CController] get Controller-Meeting Socket Data from Message Queue");
		serverMeeting->onReceive(nId, pData);
		break;
	case EVENT_COMMAND_SOCKET_TCP_DEVICE_RECEIVE:
		_log("[CController] get Device Socket Data from Message Queue");
		serverDevice->onReceive(nId, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_MEETING:
		serverMeeting->addClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT_DEVICE:
		//serverDevice->addClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_MEETING:
		serverMeeting->deleteClient(nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_DEVICE:
		//serverDevice->deleteClient(nId);
		break;
	default:
		_log("[Controller] Unknown message command: %d", nCommand);
		break;
	}

}

int CController::startServerMeeting(string strIP, const int nPort, const int nMsqId)
{
	serverMeeting->setCallback(CB_DEVCIE_COMMAND, IonDeviceCommand);
	return serverMeeting->startServer(strIP, nPort, nMsqId);
}

int CController::startServerDevice(string strIP, const int nPort, const int nMsqId)
{
	serverDevice->setCallback(CB_MEETING_COMMAND, IonMeetingCommand);
	return serverDevice->startServer(strIP, nPort, nMsqId);
}

int CController::startCmpWordServer(int nPort, int nMsqKey)
{
	int nResult = FALSE;

	if (0 != cmpword)
	{
		delete cmpword;
		cmpword = 0;
	}

	cmpword = new CCmpWord();
	cmpword->start(0, nPort, nMsqKey);
	nResult = TRUE;

	return nResult;
}
