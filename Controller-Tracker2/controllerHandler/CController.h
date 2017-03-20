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
class CServerAccessLog;
class CSocket;
class CClientControllerMongoDB;

class CController: public CObject
{
public:
	virtual ~CController();
	static CController* getInstance();
	int startServerAccesslog(std::string strIP, const int nPort);
	void stopServerAccesslog();
	void onMongoDBCommand(const void * param);
	int startClientMongoDB(std::string strIP, const int nPort, const int nMsqId);
	void runEnquireLinkRequest();
	CCmpHandler *cmpParser;

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();

	int reStartClientMongoDB();
	int sendCommand(int commandID, int seqNum);
	int cmpEnquireLinkRequest(const int nSocketFD);
	CServerAccessLog * cmpAccesslog;

	CThreadHandler *tdEnquireLink;
	//std::vector<int> vEnquireLink;
	CClientControllerMongoDB *clientMongo;

	std::string clientMongoDBIP;
	int clientMongoDBPort;
	int clientMongoDBMsqId;
};
