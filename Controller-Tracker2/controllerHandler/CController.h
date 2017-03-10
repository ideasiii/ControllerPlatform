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



using namespace std;

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
	int startServerDevice(string strIP, const int nPort, const int nMsqId);
	void stopServer();
	void onMongoDBCommand(void * param);
	int startClientMongoDB(string strIP, const int nPort, const int nMsqId);
	void runEnquireLinkRequest();

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();
	string clientMongoDBIP = "";
	int clientMongoDBPort = -1;
	int  clientMongoDBMsqId = -1;
	int reStartClientMongoDB();
	int sendCommand(int commandID, int seqNum);
	int cmpEnquireLinkRequest(const int nSocketFD);


public:
	CCmpHandler *cmpParser;

private:
	CServerDevice *serverDevice;
	CThreadHandler *tdEnquireLink;
	CThreadHandler *tdExportLog;
	std::vector<int> vEnquireLink;

	CClientControllerMongoDB *clientMongo;


};
