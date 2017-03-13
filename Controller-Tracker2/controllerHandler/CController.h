/*
 * CController.h
 *
 *  Created on: 2016年06月27日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>

#include "CObject.h"
#include "common.h"
#include "packet.h"

class CCmpHandler;
class CThreadHandler;
class CJsonHandler;
class CServerDevice;
class CSocket;
class CClientControllerMongoDB;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int startServerDevice(std::string strIP, const int nPort, const int nMsqId);
	void stopServer();
	void onMongoDBCommand(void * param);
	int startClientMongoDB(std::string strIP, const int nPort, const int nMsqId);
	void runEnquireLinkRequest();

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();
	std::string clientMongoDBIP;
	int clientMongoDBPort;
	int clientMongoDBMsqId;
	int reStartClientMongoDB();
	int sendCommand(int commandID, int seqNum);
	int cmpEnquireLinkRequest(const int nSocketFD);

public:
	CCmpHandler *cmpParser;

private:
	CServerDevice *serverDevice;
	CThreadHandler *tdEnquireLink;
	std::vector<int> vEnquireLink;
	CClientControllerMongoDB *clientMongo;

};
