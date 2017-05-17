/*
 * CServerAMX.h
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#pragma once

#include "CCmpServer.h"
#include <string>
#include <map>
#include "CMPData.h"

#include "CThreadHandler.h"
#include "iCommand.h"
class CCmpHandler;

using namespace std;

class CServerMeeting: public CCmpServer
{
public:
	CServerMeeting(CObject *object);
	~CServerMeeting();
	void addClient(const int nSocketFD);
	void deleteClient(const int nSocketFD);
	int sendCommand(int commandID, int seqNum, string bodyData);
	void setCallback(const int nId, CBFun cbfun);
	void runEnquireLinkRequest();
	int controllerCallBack(int nSocketFD, int nDataLen, const void *pData);
	int onResponse(int nSocket, int nCommand, int nStatus, int nSequence, const void *szBody);

protected:
	//void onClientDisconnect(unsigned long int nSocketFD);
private:

	int onBind(int nSocket, int nCommand, int nSequence, const void *szBody);
	int onUnbind(int nSocket, int nCommand, int nSequence, const void *pData);


	CMPData parseCMPData(int nSocket, int nCommand, int nSequence, const void *szBody);

	int cmpEnquireLinkRequest(const int nSocketFD);
	int getBindSocket(vector<int> &listValue);

	int cmpEnquireLinkResponse(int nSocket, int nCommand, int nSequence, const void *pData);

	vector<int> mapClient;
	map<int, CBFun> mapCallback;
	CThreadHandler *tdEnquireLink;
	CObject * mpController;

};
