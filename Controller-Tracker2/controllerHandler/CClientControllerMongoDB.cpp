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

CClientControllerMongoDB::CClientControllerMongoDB(CObject * object) :
		cmpParser(CCmpHandler::getInstance()), dynamicField(new DynamicField)
{
	mpController = object;
	dynamicField->setMySQLInfo("175.98.119.121", "tracker", "ideas123!", "ideas", "field");
}

CClientControllerMongoDB::~CClientControllerMongoDB()
{

}
/*
int CClientControllerMongoDB::onAccesslogResponse(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	//deal with response
	_log("[CClientControllerMongoDB]onAccesslogResponse");
	return TRUE;
}

int CClientControllerMongoDB::onEnquireLinkResponse(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_log("[CClientControllerMongoDB] Get Enquire Link Response!");
	return 1;
}
*/

int CClientControllerMongoDB::onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody)
{
	_log("[CClientControllerMongoDB] Get Response!!");
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

