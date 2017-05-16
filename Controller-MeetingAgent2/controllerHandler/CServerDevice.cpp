#include "CServerDevice.h"
#include "IReceiver.h"
#include "event.h"
#include "common.h"
#include "ICallback.h"
#include "iCommand.h"
#include "JSONObject.h"
#include "packet.h"

CServerDevice::CServerDevice(CObject *object)
{
	mpController = object;
}

CServerDevice::~CServerDevice()
{
}

int CServerDevice::onFCMIdRegister(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpFCMIdRegister");

	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);

	return TRUE;
}

int CServerDevice::onFBToken(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpFBToken");

	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);

	return TRUE;
}

int CServerDevice::onWirelessPowerCharge(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpWirelessPowerCharge");
	string chargeData = "{\"CHARGE_LOCATION\": \"1\"}";

	response(nSocket, nCommand, STATUS_ROK, nSequence, chargeData.c_str());

	return TRUE;
}

int CServerDevice::onQRCodeToken(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpQRCodeToken");

	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, szBody);
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
	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, szBody);
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
	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, szBody);
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


void CServerDevice::sendCommand(int socketFD, int commandID, int seqNum, std::string bodyData)
{

	response(socketFD, commandID, STATUS_ROK, seqNum, bodyData.c_str());


}

int CServerDevice::onAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_DBG("[CServerDevice]cmpAMXControlAcess");

	CMPData mCMPData = parseCMPData(nSocket, nCommand, nSequence, szBody);
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

CMPData CServerDevice::parseCMPData(int nSocket, int nCommand, int nSequence, const void *szBody)
{

	const char *pBody = reinterpret_cast<const char*>(szBody);


	if (pBody)
	{
		return CMPData(nSocket, nCommand, nSequence, pBody);
	}
	else
	{
		return CMPData(nSocket, nCommand, nSequence, "");
	}

}

void CServerDevice::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

