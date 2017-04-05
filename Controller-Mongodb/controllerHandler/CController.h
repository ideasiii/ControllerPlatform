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

class CMongoDBHandler;
class CTrackerServer;

class CController: public CObject
{

public:
	virtual ~CController();
	static CController* getInstance();
	int startTrackerServer(const char *szIP, const int nPort);
	int stop();

	int startServer(const int nPort);
	void stopServer();
	void onClientCMP(int nClientFD, int nDataLen, const void *pData);

protected:
	void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	explicit CController();

	std::string insertLog(const int nType, std::string strData);

private:
	CTrackerServer *trackerServer;
};
