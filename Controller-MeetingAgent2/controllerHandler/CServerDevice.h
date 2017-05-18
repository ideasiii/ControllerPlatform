#pragma once

#include <string>
#include <map>

#include "CCmpServer.h"
#include "CMPData.h"
#include "ICallback.h"
#include "iCommand.h"



class CServerDevice: public CCmpServer
{
public:

	explicit CServerDevice(CObject *object);
	~CServerDevice();
	void setCallback(const int nId, CBFun cbfun);
	//for other Controller Data

	int onFCMIdRegister(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onFBToken(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onWirelessPowerCharge(int nSocket, int nCommand, int nSequence, const void *szBody);

	//for Controller-Meeting Data
	int onQRCodeToken(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onAPPVersion(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onGetMeetingData(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onAMXControlAccess(int nSocket, int nCommand, int nSequence, const void *szBody);
	void sendCommand(int socketFD, int commandID, int seqNum, std::string bodyData);

private:


	CMPData parseCMPData(int nSocket, int nCommand, int nSequence, const void *pData);
	CObject * mpController;
	map<int, CBFun> mapCallback;

};
