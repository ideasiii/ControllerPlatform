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

#include "CApplication.h"
#include "common.h"
#include "packet.h"

class CCmpHandler;
class CThreadHandler;
class CJsonHandler;
class CServerAccessLog;
class CSocket;
class CClientControllerMongoDB;

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();
	void runEnquireLinkRequest();
protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);

private:

	void setClientMongoValues(std::string, int ,int);
	int cmpEnquireLinkRequest(const int nSocketFD);
	int startServerAccesslog(std::string strIP, const int nPort, const int);

	void onMongoDBCommand(const void * param);
	int startClientMongoDB();



	CServerAccessLog * cmpAccesslog;
	CThreadHandler *tdEnquireLink;
	CClientControllerMongoDB *clientMongo;

	std::string clientMongoIP;
	int clientMongoPort;
	int clientMongoMsqId;
	bool clientMongoInit;

	bool isEquireLinkThreadStart;

	int mnMsqKey;
};
