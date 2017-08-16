/*
 * CController.cpp
 *
 *  Created on: 2016年06月27日
 *      Author: joe
 */

#include <list>
#include <ctime>

#include "CSocket.h"
#include "CConfig.h"
#include "event.h"
#include "utility.h"
#include "CCmpHandler.h"
#include "CController.h"
#include "CDataHandler.cpp"
#include "CThreadHandler.h"
#include "CClientControllerMongoDB.h"
#include "CServerAccessLog.h"
#include "iCommand.h"
#include "JSONObject.h"
#include "LogHandler.h"

#define ENQUIRE_LINK_TIME 10

using namespace std;

/** Enquire link function declare for enquire link thread **/
void *threadEnquireLinkRequest(void *argv);

CController::CController() :
		cmpAccesslog(0), tdEnquireLink(0), clientMongo(0), clientMongoPort(-1), clientMongoMsqId(-1), clientMongoInit(
				false), isEquireLinkThreadStart(false), mnMsqKey(-1)
{
}

CController::~CController()
{
}

int CController::onCreated(void* nMsqKey)
{
	cmpAccesslog = new CServerAccessLog(this);
	clientMongo = new CClientControllerMongoDB(this);
	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_TRACKER;
	return mnMsqKey;
}
int CController::onInitial(void* szConfPath)
{
	int nAccessLogServerPort;
	int nMongoClientPort;
	string strAccessLogServerPort;
	CConfig *config;
	string strConfPath;
	string strMongoClientIP;
	string strMongoClientPort;

	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if (strConfPath.empty())
		return FALSE;

	config = new CConfig();
	if (config->loadConfig(strConfPath))
	{
		strAccessLogServerPort = config->getValue("SERVER ACCESSLOG", "port");
		if (!strAccessLogServerPort.empty())
		{
			convertFromString(nAccessLogServerPort, strAccessLogServerPort);
			if (startServerAccesslog("", nAccessLogServerPort, mnMsqKey) == TRUE)
			{

			}
			else
			{
				//start Server AccessLog Fail
				_log("[CController] onInitial Start Server AccessLog Fail ");
			}
		}

		strMongoClientPort = config->getValue("CLIENT MONGOCONTROLLER", "port");
		strMongoClientIP = config->getValue("CLIENT MONGOCONTROLLER", "ip");
		if (!strMongoClientIP.empty() && !strMongoClientPort.empty())
		{
			_log("########strMongoClientPort: %s   strMongoClientIP: %s", strMongoClientPort.c_str(),
					strMongoClientIP.c_str());
			convertFromString(nMongoClientPort, strMongoClientPort);

			setClientMongoValues(strMongoClientIP, nMongoClientPort, mnMsqKey);

			//setClientMongoValues(strMongoClientIP, nMongoClientPort, mnMsqKey);

			if (startClientMongoDB() == TRUE)
			{

			}
			else
			{
				//Start Client Mongo Fail
				_log("[CController] onInitial Start Client Mongo Fail ");
			}
		}

	}
	delete config;
	return TRUE;
}
int CController::onFinish(void* nMsqKey)
{
	if (cmpAccesslog)
	{
		delete cmpAccesslog;
		cmpAccesslog = 0;
	}

	if (clientMongo)
	{
		delete clientMongo;
		clientMongo = 0;
	}

	return TRUE;
}

void CController::onHandleMessage(Message &message)
{
	int nRet;
	switch (message.what)
	{
	//handle device client message
	case MESSAGE_EVENT_DEVICE_SERVER:
		//_log("[controller] get device message: %s", message.strData.c_str());
		nRet = clientMongo->sendCommand((void *) message.strData.c_str());
		if (nRet < 1)
			_log("[controller] some error while sending to client Mongo");
		break;
	case MESSAGE_EVENT_MONGO_CLIENT:

		break;
	default:
		break;

	}
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	int status = 0;
	_log("[CController] onReceiveMessage");
	switch (nCommand)
	{

	case EVENT_COMMAND_RECONNECT_CONTROLLER_MONGODB:

		_log("[CController]***Start to ReConnect Controller-MongoDB***\n");

		if (!clientMongo)
		{
			clientMongo = new CClientControllerMongoDB(this);
		}
		status = startClientMongoDB();

		_log("[CController] ReConnect Controller-MongoDB, status: %d\n", status);
		break;

	default:
		_log("[Controller] Unknown Message Command: %d", nCommand);
		break;
	}
}

int CController::startServerAccesslog(string strIP, const int nPort, const int nMKey)
{
	_log("[Controller] AccessLog IP: %s port: %d", strIP.c_str(), nPort);
	if (!strIP.empty())
	{
		if (cmpAccesslog->start(strIP.c_str(), nPort, nMKey))
		{
			cmpAccesslog->idleTimeout(true, 5);
			return TRUE;
		}
	}
	else
	{
		if (cmpAccesslog->start(0, nPort, nMKey))
		{
			cmpAccesslog->idleTimeout(true, 5);
			return TRUE;
		}
	}
	return FALSE;
}

void CController::setClientMongoValues(string ip, int port, int key)
{
	this->clientMongoIP = ip;
	this->clientMongoPort = port;
	this->clientMongoMsqId = key;
	this->clientMongoInit = true;
}

int CController::startClientMongoDB()
{
	if (clientMongoInit == false)
	{
		_log("[Controller] ClientMongo not Init!");
		return 0;
	}

	_log("[Controller] start to connect IP: %s, port: %d \n", clientMongoIP.c_str(), clientMongoPort);

	if (clientMongo->connect(clientMongoIP.c_str(), clientMongoPort, clientMongoMsqId) > 0)
	{
		if (!isEquireLinkThreadStart)
		{
			isEquireLinkThreadStart = true;
			tdEnquireLink->createThread(threadEnquireLinkRequest, this);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

void CController::runEnquireLinkRequest()
{
	while (1)
	{
		tdEnquireLink->threadSleep(ENQUIRE_LINK_TIME);
		_log("[CController] Enquire Link Start");
		if (!clientMongo)
		{
			_log("[CController] client Mogno is not init!");
			break;
		}
		if (clientMongo->isValidSocketFD())
		{
			int nRet = cmpEnquireLinkRequest(clientMongo->getSocketfd());

			if (nRet > 0)
			{
				//Enquire Link Success
			}
			else
			{
				//Enquire Link Failed
				_log("[CController] Send Enquire Link Failed result = %d\n", nRet);
				int status = sendMessage(EVENT_FILTER_CONTROLLER,
				EVENT_COMMAND_RECONNECT_CONTROLLER_MONGODB, 0, 0, NULL);
			}
		}
		else
		{
			_log("[CController] ERROR to find Controller-MongoDB Socket ID!\n");

			sendMessage(EVENT_FILTER_CONTROLLER, EVENT_COMMAND_RECONNECT_CONTROLLER_MONGODB, 0, 0,
			NULL);

		}

	}

}

int CController::cmpEnquireLinkRequest(const int nSocketFD)
{
	int nRet = 0;
	if (clientMongo->isValidSocketFD())
	{
		nRet = sendPacket(dynamic_cast<CSocket*>(clientMongo), clientMongo->getSocketfd(), enquire_link_request,
		STATUS_ROK, getSequence(), 0);
	}
	return nRet;
}

/************************************* thread function **************************************/
void *threadEnquireLinkRequest(void *argv)
{
	CController* ss = reinterpret_cast<CController*>(argv);
	ss->runEnquireLinkRequest();
	return NULL;
}

