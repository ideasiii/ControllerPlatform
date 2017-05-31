#include <stdio.h>
#include "CController.h"
#include "common.h"
#include "CConfig.h"
#include "utility.h"
#include "event.h"
#include "packet.h"
#include "CServerDevice.h"
#include "CServerMeeting.h"
#include "iCommand.h"
#include "JSONObject.h"
#include <string>

using namespace std;

static CController *mCController = 0;

void IonMeetingCommand(void *param)
{
	const CMPData *strParam = reinterpret_cast<const CMPData*>(param);
	mCController->onMeetingCommand(strParam);
}

void IonDeviceCommand(void *param)
{
	const CMPData *strParam = reinterpret_cast<const CMPData*>(param);
	mCController->onDeviceCommand(strParam);
}

CController::CController() :
		serverMeeting(0), serverDevice(0), mnMsqKey(-1)
{
}

CController::~CController()
{
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
	mCController = this;
	string strConfPath = reinterpret_cast<const char*>(szConfPath);
	if (strConfPath.empty())
	{
		return FALSE;
	}
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
			startServerDevice("", nPort, -1);
		}

		strControllerMeetingPort = config->getValue("SERVER CONTROLLER_METTING", "port");
		if (!strControllerMeetingPort.empty())
		{
			convertFromString(nPort, strControllerMeetingPort);
			startServerMeeting("", nPort, -1);
		}

	}
	delete config;
	return TRUE;
}

int CController::onFinish(void* nMsqKey)
{

	return TRUE;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{

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

void CController::onHandleMessage(Message &message)
{
	int nRet;
	_log("[controller] get onHandleMessage: %s", message.strData.c_str());

	switch (message.what)
	{
	//handle device client message
	case MESSAGE_EVENT_DEVICE_SERVER:

		switch (message.arg[0])
		{
		case MESSAGE_FILITER_FB_TOKEN:
			//call http client to send data to SER


			break;

		case MESSAGE_FILITER_FCM_ID:
			//call FCM Controller to send data



			break;
		default:



			break;

		}

		break;
	default:
		break;

	}
}

int CController::startServerMeeting(string strIP, const int nPort, const int nMsqId)
{
	serverMeeting = new CServerMeeting(this);

	serverMeeting->setCallback(CB_DEVCIE_COMMAND, IonDeviceCommand);

	if (!strIP.empty())
	{
		return serverMeeting->start(strIP.c_str(), nPort, nMsqId);
	}
	else
	{
		return serverMeeting->start(0, nPort, nMsqId);
	}
}

int CController::startServerDevice(string strIP, const int nPort, const int nMsqId)
{
	serverDevice = new CServerDevice(this);

	serverDevice->setCallback(CB_MEETING_COMMAND, IonMeetingCommand);

	if (!strIP.empty())
	{
		return serverDevice->start(strIP.c_str(), nPort, nMsqId);
	}
	else
	{
		return serverDevice->start(0, nPort, nMsqId);
	}

}

