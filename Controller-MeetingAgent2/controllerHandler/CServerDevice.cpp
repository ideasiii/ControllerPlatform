#include "CServerDevice.h"
#include "IReceiver.h"
#include "event.h"
#include "common.h"
#include "CCmpHandler.h"
#include "ICallback.h"
#include "CDataHandler.cpp"
#include "iCommand.h"
#include "JSONObject.h"
#include "packet.h"


CServerDevice::CServerDevice()
{
	this->setUseQueueReceive(true);
}

CServerDevice::~CServerDevice()
{
	mapClient.clear();
}

void CServerDevice::sendCommand(int socketFD, int commandID, int seqNum, string bodyData)
{
	_log("[CServerDevice] Send Command back to Device socketFD:%d, command:%d, seqNum:%d, data:%s\n", socketFD,
			commandID, seqNum, bodyData.c_str());

	sendPacket(socketFD, commandID, STATUS_ROK, seqNum, bodyData.c_str());

}
int CServerDevice::onFCMIdRegister(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpFCMIdRegister");

	sendPacket(nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, 0);

	return TRUE;
}

int CServerDevice::onFBToken(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpFBToken");

	sendPacket(nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, 0);

	return TRUE;
}

int CServerDevice::onWirelessPowerCharge(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpWirelessPowerCharge");
	string chargeData = "{\"CHARGE_LOCATION\": \"1\"}";

	sendPacket(nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, chargeData.c_str());

	return TRUE;
}

int CServerDevice::onQRCodeToken(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpQRCodeToken");

	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, szBody, true);
	if (mCMPData.isVaild())
	{
		(*mapCallback[CB_MEETING_COMMAND])(static_cast<void*>(const_cast<CMPData*>(&mCMPData)));
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

int CServerDevice::onAPPVersion(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpAPPVersion");
	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, szBody, false);
	if (mCMPData.isVaild())
	{
		(*mapCallback[CB_MEETING_COMMAND])(static_cast<void*>(const_cast<CMPData*>(&mCMPData)));
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

int CServerDevice::onGetMeetingData(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpGetMeetingData");
	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, szBody, true);
	if (mCMPData.isVaild())
	{
		(*mapCallback[CB_MEETING_COMMAND])(static_cast<void*>(const_cast<CMPData*>(&mCMPData)));
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

int CServerDevice::onAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpAMXControlAcess");

	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, szBody, true);
	if (mCMPData.isVaild())
	{
		(*mapCallback[CB_MEETING_COMMAND])(static_cast<void*>(const_cast<CMPData*>(&mCMPData)));
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

CMPData CServerDevice::parseCMPData(int nSocket, int nCommand, int nSequence, const void *szBody, bool isBodyExist)
{
	string *data = static_cast<const string*>(szBody);

	string s = *data;
	delete data;

	if (s.size() > 0)
	{
		return CMPData(nSocket, nCommand, nSequence, s);
	}
	else if (isBodyExist == false)
	{
		return CMPData(nSocket, nCommand, nSequence, "");
	}
	else
	{
		return CMPData(-1, -1, -1, NULL);
	}
}

void CServerDevice::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

