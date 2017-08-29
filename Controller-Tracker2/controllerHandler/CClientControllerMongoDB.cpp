#include "IReceiver.h"
#include "event.h"
#include "utility.h"
#include "common.h"
#include "CClientControllerMongoDB.h"
#include "JSONObject.h"
#include "packet.h"
#include "DynamicField.h"

CClientControllerMongoDB::CClientControllerMongoDB(CObject * object) :
		dynamicField(new DynamicField)
{
	mpController = object;
	dynamicField->setMySQLInfo("127.0.0.1", "tracker", "ideas123!", "ideas", "field");
}

CClientControllerMongoDB::~CClientControllerMongoDB()
{

}

int CClientControllerMongoDB::onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody)
{
	//_log("[CClientControllerMongoDB] Get AccessLog Response!! Status:%d Sequence: %d ", nStatus, nSequence);
	printPacket(nCommand, nStatus, nSequence, 16, "[CClientControllerMongoDB] onResponse Response ", nSocket);
	return 0;
}

int CClientControllerMongoDB::sendCommand(const void * param)
{

	const char* data = static_cast<const char*>(param);

	int nRet;

	if (dynamicField->isValidJSONFormat(string(const_cast<char *>(data))) == true)
	{
		dynamicField->insertDynamicData(string(const_cast<char *>(data)));
		//dynamicField->printAllCaches();
		nRet = cmpAccessLogRequest("", string(const_cast<char *>(data)));
	}
	else
	{
		_log("[CClientControllerMongoDB] is Not Valid JSON Format: %s", data);
	}

	return nRet;

}

int CClientControllerMongoDB::cmpAccessLogRequest(string strType, string strLog)
{
	int nRet = -1;
	if (!isValidSocketFD())
	{
		return nRet;
	}

	_DBG("[CClientControllerMongoDB] use Socket send AccessLog to MongoDB Controller");

	nRet = request(getSocketfd(), access_log_request, STATUS_ROK, getSequence(), strLog.c_str());

	return nRet;
}

