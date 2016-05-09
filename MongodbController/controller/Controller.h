/*
 * Controller.h
 *
 *  Created on: 2016年5月9日
 *      Author: Jugo
 */

#pragma once

#include <string>
#include <vector>
#include <list>
#include "CObject.h"

class CSocketServer;
class CCmpHandler;
class CMongoDBHandler;

class Controller: public CObject
{

	public:
		virtual ~Controller();
		static Controller* getInstance();
		int init(std::string strConf);
		int startServer(const int nPort);
		void stopServer();
		void receiveClientCMP(int nClientFD, int nDataLen, const void *pData);

	protected:
		void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

	private:
		Controller();
		void onClientCMP(int nClientFD, int nDataLen, const void *pData);
		void ackPacket(int nClientSocketFD, int nCommand, const void * pData);
		int sendCommandtoClient(int nSocket, int nCommand, int nStatus, int nSequence, bool isResp);
		int cmpUnknow(int nSocket, int nCommand, int nSequence, const void * pData);
		int cmpAccessLog(int nSocket, int nCommand, int nSequence, const void *pData);

	private:
		CSocketServer *cmpServer;		// controller message protocol server
		CCmpHandler *cmpParser;
		CMongoDBHandler *mongodb;
};
