#pragma once

#include <string>
#include <map>

#include "CSocketServer.h"
#include "CMPData.h"
#include "ICallback.h"
#include "iCommand.h"

using namespace std;


class CServerDevice: public CCmpServer
{
public:

	CServerDevice();
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

private:

	map<int, MemFn> mapFunc;

	CMPData parseCMPData(int nSocket, int nCommand, int nSequence, const void *pData, bool isBodyExist);

	map<int, CBFun> mapCallback;

};
