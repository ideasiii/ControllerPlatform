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
#include "event.h"
#include "packet.h"
#include "CCmpHandler.h"
#include "utility.h"
#include "CDataHandler.cpp"
#include "IReceiver.h"
#include <map>
#include "CMongoDBHandler.h"

using namespace std;

static Controller * controller = 0;

/**
 * Define Socket Client ReceiveFunction
 */
int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
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

Controller::Controller() :
		CObject(), cmpServer(new CSocketServer), cmpParser(new CCmpHandler), mongodb(CMongoDBHandler::getInstance())
{

}

Controller::~Controller()
{
	delete mongodb;
	delete cmpParser;
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

	G_LOG_PATH = config->getValue("LOG", "log");
	if (G_LOG_PATH.empty())
	{
		G_LOG_PATH = "/data/opt/tomcat/webapps/logs/mongodbController.log";
	}
	mkdirp(G_LOG_PATH);
	_DBG("[Mongodb Controller] Log Path:%s", G_LOG_PATH.c_str());

	/** Server init and start **/
	string strPort = config->getValue("SERVER", "port");
	delete config;

	if (!strPort.empty())
	{
		int nPort = atoi(strPort.c_str());
		return startServer(nPort);
	}

	return FALSE;
}

void Controller::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
		case EVENT_COMMAND_SOCKET_CONTROLLER_RECEIVE:
			onClientCMP(nId, nDataLen, pData);
			break;
		default:
			printLog("unknow message command", "[Mongodb Controller]", G_LOG_PATH);
			break;
	}
}

int Controller::startServer(const int nPort)
{
	/** Run socket server for CMP **/
	//cmpServer->setPackageReceiver( MSG_ID, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_CONTROLLER_RECEIVE);
	if (0 >= nPort)
	{
		_DBG("Mongodb Controller Start Fail, Invalid Port:%d", nPort)
		return FALSE;
	}
	/** Start TCP/IP socket listen **/
	if ( FAIL == cmpServer->start( AF_INET, NULL, nPort))
	{
		_DBG("Mongodb Controller Socket Server Create Fail")
		return FALSE;
	}

	return TRUE;
}

void Controller::stopServer()
{
	if (cmpServer)
	{
		cmpServer->stop();
		delete cmpServer;
		cmpServer = 0;
	}
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
	printPacket(nCommandSend, nStatus, nSequence, nRet, "[Controller Send to Client]", G_LOG_PATH.c_str(), nSocket);
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
	_DBG("[Mongodb Controller] Unknow command:%d", nCommand)
	sendCommandtoClient(nSocket, nCommand, STATUS_RINVCMDID, nSequence, true);
	return 0;
}

int Controller::cmpAccessLog(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CDataHandler<std::string> rData;
	int nRet = cmpParser->parseBody(nCommand, pData, rData);
	if (0 < nRet && rData.isValidKey("type") && rData.isValidKey("data"))
	{
		_DBG("[Mongodb Controller] Access Log Type:%s Data:%s FD:%d", rData["type"].c_str(), rData["data"].c_str(),
				nSocket)
		sendCommandtoClient(nSocket, nCommand, STATUS_ROK, nSequence, true);
	}
	else
	{
		_DBG("[Mongodb Controller] Access Log Fail, Invalid Body Parameters Socket FD:%d", nSocket)
		sendCommandtoClient(nSocket, nCommand, STATUS_RINVBODY, nSequence, true);
	}
	rData.clear();
	return 0;
}

/**
 * 	Receive CMP from Client
 */
void Controller::onClientCMP(int nClientFD, int nDataLen, const void *pData)
{
	_DBG("[Mongodb Controller] Receive CMP From Client:%d Length:%d", nClientFD, nDataLen)

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

	printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
			"[Controller Recv]", G_LOG_PATH.c_str(), nClientFD);

	if (access_log_request != cmpHeader.command_id)
	{
		sendCommandtoClient(nClientFD, cmpHeader.command_id, STATUS_RINVCMDID, cmpHeader.sequence_number, true);
		return;
	}

	cmpAccessLog(nClientFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);
}

void Controller::receiveClientCMP(int nClientFD, int nDataLen, const void *pData)
{
	onClientCMP(nClientFD, nDataLen, pData);
}
