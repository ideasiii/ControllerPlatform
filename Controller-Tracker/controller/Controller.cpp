/*
 * Controller.cpp
 *
 *  Created on: 2015年10月19日
 *      Author: Louis Ju
 */

#include "Controller.h"
#include "common.h"
#include "Config.h"
#include "CSocketServer.h"
#include "CSocketClient.h"
#include "event.h"
#include "packet.h"
#include "CCmpHandler.h"
#include "utility.h"
#include "CDataHandler.cpp"
#include "CSqliteHandler.h"
#include "CThreadHandler.h"
#include "IReceiver.h"
#include <map>
#include "ClientHandler.h"
#include "CSqlite.h"
#include "JSONObject.h"
#include <vector>

using namespace std;

static Controller * controller = 0;

map<string, string> mapWire;

/** Enquire link function declare for enquire link thread **/
void *threadEnquireLinkRequest(void *argv);

/**
 * Define Socket Client ReceiveFunction
 */
int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
	controller->receiveCenterCMP(nSocketFD, nDataLen, pData);
	return 0;
}

/**
 *  Define Socket Server Receive Function
 */
int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
{

	controller->receiveClientCMP(nSocketFD, nDataLen, pData);
	return 0;
}

static int accessSequence = 0x00000000;
static int getAccessLogSequence()
{
	++accessSequence;
	if (0x7FFFFFFF <= accessSequence)
		accessSequence = 0x00000001;
	return accessSequence;
}

Controller::Controller() :
		CObject(), cmpServer(new CSocketServer), cmpClient(new CSocketClient), mongoDBClient(new CSocketClient), cmpParser(
				new CCmpHandler), sqlite(CSqliteHandler::getInstance()), tdEnquireLink(new CThreadHandler), clientHandler(
				ClientHandler::getInstance()), mCSqlite(CSqlite::getInstance())
{
	for (int i = 0; i < MAX_FUNC_POINT; ++i)
	{
		cmpRequest[i] = &Controller::cmpUnknow;
	}
	cmpRequest[bind_request] = &Controller::cmpBind;
	cmpRequest[unbind_request] = &Controller::cmpUnbind;
	cmpRequest[access_log_request] = &Controller::cmpAccessLog;
	cmpRequest[enquire_link_request] = &Controller::cmpEnquireLink;

	clientCollection.push_back(cmpClient);
	clientCollection.push_back(mongoDBClient);

}

Controller::~Controller()
{
	cmpClient->stop();
	mongoDBClient->stop();
	delete cmpClient;
	delete mongoDBClient;
	delete cmpParser;
	delete sqlite;
	delete mCSqlite;
}

Controller* Controller::getInstance()
{
	if (0 == controller)
	{
		controller = new Controller();
	}
	return controller;
}

int Controller::init(std::string strConf)
{

	/** Load config file **/
	Config *config = new Config();
	if ( FALSE == config->loadConfig(strConf))
	{
		_DBG("Load Config File Fail:%s", strConf.c_str());
		delete config;
		return FALSE;
	}

	G_LOG_PATH = mConfig.strLogPath = config->getValue("LOG", "log");
	if (mConfig.strLogPath.empty())
	{
		mConfig.strLogPath = "controller.log";
	}
	mkdirp(mConfig.strLogPath);
	_DBG("[Controller] Log Path:%s", mConfig.strLogPath.c_str());

	/** Get Server Port **/
	mConfig.strServerPort = config->getValue("SERVER", "port");
	_DBG("[Controller] Server Port:%s", mConfig.strServerPort.c_str());

	char *szMAC = cmpServer->getMac();
	mConfig.strMAC = szMAC;
	free(szMAC);
	_DBG("[Controller] MAC Address:%s", mConfig.strMAC.c_str());

	mConfig.strCenterServerIP = config->getValue("CENTER", "ip");
	mConfig.strCenterServerPort = config->getValue("CENTER", "port");
	_DBG("[Controller] Control Center IP:%s Port:%s", mConfig.strCenterServerIP.c_str(),
			mConfig.strCenterServerPort.c_str());

	/** Create sqlite DB device [must]**/
	string strDeviceDB = config->getValue("SQLITE", "db_device");
	if (strDeviceDB.empty())
	{
		strDeviceDB = "/data/sqlite/device.db";
	}
	mkdirp(strDeviceDB);
	if (!sqlite->openDeviceDB(strDeviceDB.c_str()))
	{
		_log("[Controller] Open Sqlite DB FAIL Path:%s",strDeviceDB.c_str());
		return FALSE;
	}

	//read ideas.db data
	string strIdeasDB = config->getValue("SQLITE", "db_ideas");
	if (strIdeasDB.empty())
	{
		strDeviceDB = "/data/sqlite/ideas.db";
	}
	mkdirp(strIdeasDB);
	if (!sqlite->openIdeasDB(strIdeasDB.c_str()))
	{
		_log("[Controller] Open Sqlite DB FAIL Path:%s",strIdeasDB.c_str());
		return FALSE;
	}

	/** Create sqlite DB Field [must]**/
	string strFieldDB = config->getValue("SQLITE", "db_field");
	if (strFieldDB.empty())
	{
		strFieldDB = "/data/sqlite/field.db";
	}
	mkdirp(strFieldDB);
	if (!sqlite->openFieldDB(strFieldDB.c_str()))
	{
		_log("[Controller] Open Sqlite DB FAIL Path:%s",strFieldDB.c_str());
		return FALSE;
	}
	_log("[Controller] Open Sqlite ALL DB Success");


	// /** Get MongoDB IP And Port **/
	mConfig.strMongoDBControllerIP = config->getValue("MONGODB", "ip");
	mConfig.strMongoDBControllerPort = config->getValue("MONGODB", "port");
	_DBG("[Controller] MongoDB Controller IP:%s Port:%s", mConfig.strMongoDBControllerIP.c_str(),
			mConfig.strMongoDBControllerPort.c_str());



	//init cmpClient message queue
	cmpClient->setPackageReceiver( MSG_ID, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_CENTER_RESPONSE);

	mongoDBClient->setPackageReceiver(MSG_ID, EVENT_FILTER_CONTROLLER,
	EVENT_COMMAND_SOCKET_CENTER_RESPONSE);

	cmpClient->setClientDisconnectCommand( EVENT_COMMAND_CONTROL_CENTER_DISCONNECT);

	mCSqlite->createMessageReceiver();

	delete config;
	return TRUE;
}

void Controller::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	case EVENT_COMMAND_SOCKET_CONTROLLER_RECEIVE:
		onClientCMP(nId, nDataLen, pData);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_CONNECT:
		_DBG("[Controller] Socket Client FD:%d Connected", (int )nId);
		break;
	case EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT:
		setUnbindState((int) nId);
		_DBG("[Controller] Socket Client FD:%d Close", (int )nId);
		break;
	case EVENT_COMMAND_CONTROL_CENTER_DISCONNECT:
		_DBG("[Controller] Control Center Dissconnect, Socket FD:%d Close", (int )nId);
		break;
	case EVENT_COMMAND_SOCKET_CENTER_RESPONSE:
		onCenterCMP(nId, nDataLen, pData);
		break;
	default:
		_DBG("[Controller] unknow message command");
		break;
	}
}

int Controller::startServer()
{
	/** Run socket server for CMP **/
	cmpServer->setPackageReceiver( MSG_ID, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_CONTROLLER_RECEIVE);
	cmpServer->setClientConnectCommand( EVENT_COMMAND_SOCKET_CLIENT_CONNECT);
	cmpServer->setClientDisconnectCommand( EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT);

	if (!mConfig.strServerPort.empty())
	{
		int nPort = atoi(mConfig.strServerPort.c_str());
		if (0 >= nPort)
		{
			_log("[Controller] CMP Server Start Fail, Invalid Port:%s", mConfig.strServerPort.c_str());
			return FALSE;
		}
		/** Start TCP/IP socket listen **/
		tdEnquireLink->createThread(threadEnquireLinkRequest, this, 0, PTHREAD_CREATE_DETACHED);
		if ( FAIL == cmpServer->start( AF_INET, NULL, nPort))
		{
			_log("[Controller] CMP Server Socket Create Fail");
			return FALSE;
		}
	}
	else
	{
		_log("[Controller] CMP Server Start Fail, Invalid Port Config");
		return FALSE;
	}

	return TRUE;
}

void Controller::stopServer()
{
	if (tdEnquireLink)
	{
		tdEnquireLink->threadExit();
		delete tdEnquireLink;
		_DBG("[Controller] Stop Enquire Link Thread");
	}

	if (cmpServer)
	{
		cmpServer->stop();
		delete cmpServer;
		cmpServer = 0;
	}
}

int Controller::connectCenter()
{
	int nRet = FAIL;
	if (mConfig.strCenterServerIP.empty() || mConfig.strCenterServerPort.empty())
	{
		_log("[Controller] Connect Control Center Fail, Config Invalid");
		return FALSE;
	}

	int nPort = atoi(mConfig.strCenterServerPort.c_str());
	if (0 >= nPort)
	{
		_log("[Controller] Connect Control Center Fail, Invalid Port:%s", mConfig.strCenterServerPort.c_str());
		return FALSE;
	}

	cmpClient->start( AF_INET, mConfig.strCenterServerIP.c_str(), nPort);
	if (cmpClient->isValidSocketFD())
	{
		_log("[Controller] Connect Center Success.");

		/*Bind to Control Center*/
		//clientHandler->setClientSocket(cmpClient);
		nRet = cmpBindRequest(cmpClient->getSocketfd(), cmpClient);
		return nRet;
	}
	else
	{
		_log("[Controller] Connect Center Fail.");
	}

	return nRet;
}

int Controller::connectMongoDBController()
{
	int nRet = FAIL;
	if (mConfig.strMongoDBControllerIP.empty() || mConfig.strMongoDBControllerPort.empty())
	{
		_log("[Controller] Connect MongoDB Controller Fail, Config Invalid");
		return FALSE;
	}

	int nPort = atoi(mConfig.strMongoDBControllerPort.c_str());
	if (0 >= nPort)
	{
		_log("[Controller] Connect MongoDB Controller Fail, Invalid Port:%s", mConfig.strMongoDBControllerPort.c_str());
		return FALSE;
	}

	mongoDBClient->start( AF_INET, mConfig.strMongoDBControllerIP.c_str(), nPort);
	if (mongoDBClient->isValidSocketFD())
	{
		_log("[Controller] mongoDB Controller link Success.");

		/*bind to mongoDB Controller*/
		return SUCCESS;
		//nRet = cmpBindRequest(mongoDBClient->getSocketfd(),mongoDBClient);

	}
	else
	{
		_log("[Controller] Connect MongoDB Controller Fail.");
	}

	return nRet;

}

int Controller::sendCommandtoSocketServer(int socketFD, int nCommand, int nStatus, int nSequence, bool isResp)
{
	int nRet = -1;
	int nCommandSend;
	CMP_HEADER cmpHeader;
	void *pHeader = &cmpHeader;

	if (socketFD != -1)
	{
		memset(&cmpHeader, 0, sizeof(CMP_HEADER));
		nCommandSend = nCommand;

		/*0x80000y is response and 0x00000y is request */
		if (isResp)
		{
			nCommandSend = generic_nack | nCommand;
		}

		cmpParser->formatHeader(nCommandSend, nStatus, nSequence, &pHeader);
		for (size_t i = 0; i < clientCollection.size(); i++)
		{
			if (clientCollection.at(i)->getSocketfd() == socketFD)
			{

				nRet = clientCollection.at(i)->socketSend(clientCollection.at(i)->getSocketfd(), &cmpHeader,
						sizeof(CMP_HEADER));
				//printPacket(nCommandSend, nStatus, nSequence, nRet, "[Controller-Tracker(sendCommandtoSocketServer)]",
				//		clientCollection.at(i)->getSocketfd());
				break;
			}
		}
	}

	return nRet;
}

/*isResp = is response*/
int Controller::sendCommandtoCenter(int nCommand, int nStatus, int nSequence, bool isResp)
{
	int nRet = -1;
	int nCommandSend;
	CMP_HEADER cmpHeader;
	void *pHeader = &cmpHeader;

	if (cmpClient->isValidSocketFD())
	{
		memset(&cmpHeader, 0, sizeof(CMP_HEADER));
		nCommandSend = nCommand;

		/*0x80000y is response and 0x00000y is request */
		if (isResp == true)
		{
			nCommandSend = generic_nack | nCommand;
		}

		cmpParser->formatHeader(nCommandSend, nStatus, nSequence, &pHeader);
		nRet = cmpClient->socketSend(cmpClient->getSocketfd(), &cmpHeader, sizeof(CMP_HEADER));
		printPacket(nCommandSend, nStatus, nSequence, nRet, "[Controller Send to Center]", cmpClient->getSocketfd());
	}

	return nRet;
}

int Controller::cmpBindRequest(const int nSocket, CSocketClient * client)
{
	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	CMP_PACKET packet;
	void *pHeader = &packet.cmpHeader;
	char *pIndex = packet.cmpBody.cmpdata;

	memset(&packet, 0, sizeof(CMP_PACKET));

	cmpParser->formatHeader( bind_request, STATUS_ROK, getSerialSequence(), &pHeader);

	memcpy(pIndex, mConfig.strMAC.c_str(), mConfig.strMAC.length());
	pIndex += mConfig.strMAC.length();
	nBody_len += mConfig.strMAC.length();
	memcpy(pIndex, "\0", 1);
	++pIndex;
	++nBody_len;

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);

	nRet = client->socketSend(nSocket, &packet, nTotal_len);
	string strMsg = "Bind to Server Controller ID:" + mConfig.strMAC + "socket FD: " + ConvertToString(nSocket);

	_log("[Controller] %s", strMsg.c_str());
	return nRet;
}

int Controller::cmpAccessLogRequest(const int nSocketFD, std::string strType, std::string strLog)
{

	int nRet = -1;
	int nBody_len = 0;
	int nTotal_len = 0;

	if(nSocketFD == -1)
			return nRet;

	CMP_PACKET packet;
	void *pHeader = &packet.cmpHeader;
	char *pIndex = packet.cmpBody.cmpdata;

	memset(&packet, 0, sizeof(CMP_PACKET));
	int accessLogSequence = getAccessLogSequence();
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

	//if (mConfig.strMongoDBControllerMsgID.empty())
	{
		_DBG("[controller] use Socket send AccessLog to MongoDB Controller");

		nRet = mongoDBClient->socketSend(nSocketFD, &packet, nTotal_len);
		printPacket( access_log_request, STATUS_ROK, accessLogSequence, nRet, "[Controller Send to Client]",
				mongoDBClient->getSocketfd());
	}
	//because of controller-tracker and mongodb controller are in the same machine
	//just send a inserted message into the mongodb controller message queue
	//else
	{
		//_DBG("[controller] use Message Queue send AccessLog to MongoDB Controller");
		//nRet = mongoDBClient->sendMessage(EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_CONTROLLER_RECEIVE, nSocketFD,
		//		nTotal_len, &packet);
	}
	return nRet;
}

int Controller::sendCommandtoClient(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp)
{
	int nRet = -1;
	int nCommandSend;
	CMP_HEADER cmpHeader;
	void *pHeader = &cmpHeader;

	memset(&cmpHeader, 0, sizeof(CMP_HEADER));
	nCommandSend = nCommand;

	if (isResp)
	{
		nCommandSend = generic_nack | nCommand;
	}

	cmpParser->formatHeader(nCommandSend, nStatus, nSequence, &pHeader);
	nRet = cmpServer->socketSend(nSocket, &cmpHeader, sizeof(CMP_HEADER));

	if (((unsigned int)nCommandSend == enquire_link_response ||(unsigned int) nCommandSend == enquire_link_request)
				&& cmpHeader.command_status == STATUS_ROK)
	{

	}
	else
	{
		printPacket(nCommandSend, nStatus, nSequence, nRet, "[Controller Send to Client]", nSocket);
	}
	return nRet;
}

void Controller::ackPacket(int nClientSocketFD, int nCommand, const void * pData)
{
	switch (nCommand)
	{
	case generic_nack:
		break;
	case bind_response:
		break;
	case authentication_response:
		break;
	case access_log_response:
		break;
	case enquire_link_response:
		for (vector<int>::iterator it = vEnquireLink.begin(); it != vEnquireLink.end(); ++it)
		{
			if (nClientSocketFD == *it)
			{
				vEnquireLink.erase(it);
				_log("[Controller] Keep alive Socket FD:%d", nClientSocketFD);
				break;
			}
		}
		break;
	case unbind_response:
		break;
	case update_response:
		break;
	case reboot_response:
		break;
	case config_response:
		break;
	case power_port_response:
		break;
	}
}

int Controller::cmpUnknow(int nSocket, int nCommand, int nSequence, const void * pData)
{
	_log("[Controller] Unknow command:%d", nCommand);
	sendCommandtoClient(nSocket, nCommand, STATUS_RINVCMDID, nSequence, true);
	return 0;
}

int Controller::cmpBind(int nSocket, int nCommand, int nSequence, const void * pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet)
	{
		_log("[Controller] Bind Get Controller ID:%s Socket FD:%d", rData["id"].c_str(), nSocket);
		string strSql = "DELETE FROM device WHERE id = '" + rData["id"] + "';";
		sqlite->deviceSqlExec(strSql.c_str());

		const string strSocketFD = ConvertToString(nSocket);
		strSql = "INSERT INTO device(id, status, socket_fd, created_date)values('" + rData["id"] + "',1," + strSocketFD
				+ ",datetime());";
		sqlite->deviceSqlExec(strSql.c_str());
		sendCommandtoClient(nSocket, nCommand, STATUS_ROK, nSequence, true);
	}
	else
	{
		_log("[Controller] Bind Fail, Invalid Controller ID Socket FD:%d", nSocket);
		sendCommandtoClient(nSocket, nCommand, STATUS_RINVCTRLID, nSequence, true);
	}
	rData.clear();
	return 0;
}

int Controller::cmpUnbind(int nSocket, int nCommand, int nSequence, const void * pData)
{
	setUnbindState(nSocket);
	sendCommandtoClient(nSocket, nCommand, STATUS_ROK, nSequence, true);
	return 0;
}

int Controller::cmpEnquireLink(int nSocket, int nCommand, int nSequence, const void *pData)
{
	return sendCommandtoClient(nSocket, nCommand, STATUS_ROK, nSequence, true);
}

int Controller::cmpAccessLog(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);

	JSONObject  mJSONObject = JSONObject((rData["data"].c_str()));
	if(mJSONObject.isValid())
	{
		if (0 < nRet && rData.isValidKey("type") && rData.isValidKey("data"))
		{
			_log("[Controller] Access Log Type:%s Data:%s FD:%d", rData["type"].c_str(), rData["data"].c_str(), nSocket);


			int result = cmpAccessLogRequest(mongoDBClient->getSocketfd(), rData["type"], rData["data"]);
			if (result > 0)
			{
				_log("[Controller] AccessLog send to MongoDB Controller success result = %d", result);
			}
			else
			{
				_log("[Controller] AccessLog send to MongoDB Controller FAIL!");
			}

			// lock and auto add device id and device field
			//mCSqlite->sendMessage(EVENT_FILTER_CSQLITE, 1, 0, rData["data"].capacity(), (void *) (rData["data"]).c_str());
			mCSqlite->createThread(rData["data"]);
			// unlock

		}
		else
		{
			_log("[Controller] Access Log Fail, Invalid Body Parameters Socket FD: %d", nSocket);
			//sendCommandtoClient(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
		}
	}
	else
	{
		_log("[Controller] Invaild JSON format!!  data: %s",rData["data"].c_str());
	}
	rData.clear();

	mJSONObject.release();

	return 0;
}

void Controller::setUnbindState(int nSocketFD)
{
	string strSql = "UPDATE device set status = 0 , updated_date = datetime() WHERE socket_fd = "
			+ ConvertToString(nSocketFD) + ";";
	sqlite->deviceSqlExec(strSql.c_str());
}
/**
 * 	Receive CMP from Client
 */
void Controller::onClientCMP(int nClientFD, int nDataLen, const void *pData)
{
	_DBG("[Controller] Receive CMP From Client:%d Length:%d", nClientFD, nDataLen);

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

	if(((unsigned int)cmpHeader.command_id == enquire_link_request || (unsigned int)cmpHeader.command_id == enquire_link_response) && cmpHeader.command_status == STATUS_ROK)
	{

	}
	else
	{
		printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
						"[Controller-Tracker(Receive CMP from Client)]", nClientFD);
	}
	if (cmpParser->isAckPacket(cmpHeader.command_id))
	{
		ackPacket(nClientFD, cmpHeader.command_id, pPacket);
		return;
	}

	if (0x000000FF < cmpHeader.command_id)
	{
		sendCommandtoClient(nClientFD, cmpHeader.command_id, STATUS_RINVCMDID, cmpHeader.sequence_number, true);
		return;
	}

	(this->*this->cmpRequest[cmpHeader.command_id])(nClientFD, cmpHeader.command_id, cmpHeader.sequence_number,
			pPacket);
}

void Controller::receiveCenterCMP(int nServerFD, int nDataLen, const void *pData)
{
	onCenterCMP(nServerFD, nDataLen, pData);
}

void Controller::receiveClientCMP(int nClientFD, int nDataLen, const void *pData)
{
	onClientCMP(nClientFD, nDataLen, pData);
}

/**
 * 	Receive CMP from Control Center
 */
void Controller::onCenterCMP(int nServerFD, int nDataLen, const void *pData)
{
	_DBG("[Controller] Receive CMP From Control Center:%d Length:%d", nServerFD, nDataLen);
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

	if (((unsigned int)cmpHeader.command_id == enquire_link_response ||(unsigned int) cmpHeader.command_id == enquire_link_request)
			&& cmpHeader.command_status == STATUS_ROK)
	{
		//_DBG("[Controller-Tracker] Command=%-20s Status=%-20s Sequence=%d Length=%d [Socket FD=%d]",
		//		cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
		//		nServerFD);
	}
	else
	{
		printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
				"[Controller-Tracker(receive CMP from Control Center)]", nServerFD);
	}
	if (cmpParser->isAckPacket(cmpHeader.command_id))
	{
		ackPacket(nServerFD, cmpHeader.command_id, pPacket);
		return;
	}

	if (0x000000FF < cmpHeader.command_id)
	{
		sendCommandtoClient(nServerFD, cmpHeader.command_id, STATUS_RINVCMDID, cmpHeader.sequence_number, true);
		return;
	}

	(this->*this->cmpRequest[cmpHeader.command_id])(nServerFD, cmpHeader.command_id, cmpHeader.sequence_number,
			pPacket);
}

int Controller::cmpEnquireLinkRequest(const int nSocketFD)
{
	return sendCommandtoSocketServer(nSocketFD, enquire_link_request, STATUS_ROK, getSerialSequence(), false);
}

void Controller::runEnquireLinkRequest()
{
	int nSocketFD = -1;
	list<int> listValue;
	string strSql;
	string strLog;

	while (1)
	{
		tdEnquireLink->threadSleep(60);
		strLog = "Run Enquire Link Request";
		_DBG("[Controller] %s", strLog.c_str());
		/** Check Enquire link response **/
		if (vEnquireLink.size())
		{
			/** Close socket that doesn't deliver enquire link response within 10 seconds **/
			for (vector<int>::iterator it = vEnquireLink.begin(); it != vEnquireLink.end(); ++it)
			{
				strSql = "DELETE FROM device WHERE socket_fd = " + ConvertToString(*it) + ";";
				sqlite->deviceSqlExec(strSql.c_str());
				close(*it);
				strLog = "Dropped connection, Close socket file descriptor filedes = " + ConvertToString(*it);
				_log("[Controller]%s", strLog.c_str());
			}
		}
		vEnquireLink.clear();

		/*for bind */
		if (0 < getBindSocket(listValue))
		{
			_DBG("WWWWWWW[connect to Contorol Center]WWWWWW");
			for (list<int>::iterator i = listValue.begin(); i != listValue.end(); ++i)
			{
				nSocketFD = *i;
				vEnquireLink.push_back(nSocketFD);
				_DBG("*********[connect to Contorol Center]******");
				cmpEnquireLinkRequest(nSocketFD);
			}
			listValue.clear();
		}

		//_DBG("*********Cmp Client socket fd %d", cmpClient->getSocketfd());
		_DBG("*********MongoDB Client socket fd %d", mongoDBClient->getSocketfd());

		/**  Check Control Center Connection **/
		/*if (!cmpClient->isValidSocketFD())
		{
			_log("[Controller] Control Center Disconnect will reconnect");
			connectCenter();
		}
		else
		{
			cmpEnquireLinkRequest(cmpClient->getSocketfd());
		}*/


		/**  Check MongoDB Controller Connection **/
		if (!mongoDBClient->isValidSocketFD())
		{
			_log("[Controller] MongoDB Controller Disconnect will reconnect");
			connectMongoDBController();
		}
		else
		{
			/*for no bind*/
			cmpEnquireLinkRequest(mongoDBClient->getSocketfd());
		}

	}

}

int Controller::getBindSocket(list<int> &listValue)
{
	string strSql = "SELECT socket_fd FROM device WHERE status = 1;";
	return sqlite->getDeviceColumeValueInt(strSql.c_str(), listValue, 0);
}

/***************** Thread Function *********************/
void *threadEnquireLinkRequest(void *argv)
{
	Controller* ss = reinterpret_cast<Controller*>(argv);
	ss->runEnquireLinkRequest();
	return NULL;
}

