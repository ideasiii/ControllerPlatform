/*
 * CAMXServer.h
 *
 *  Created on: 2017年5月15日
 *      Author: root
 */

#pragma once

#include <map>
#include "CATcpServer.h"

class CAMXServer: public CATcpServer
{
public:
	explicit CAMXServer();
	virtual ~CAMXServer();
	int request(const int nSocketFD, const char *szData);
	int response(const int nSocketFD, const char *szData);

protected:
	void onTimer(int nId);
	int onTcpReceive(unsigned long int nSocketFD);
	virtual void onClientConnect(unsigned long int nSocketFD);
	virtual void onClientDisconnect(unsigned long int nSocketFD);
	virtual int onAmxStatus(unsigned long int nSocketFD, const char *szStatus) = 0;

private:
	void bind(const int nSocketFD);
	void unbind(const int nSocketFD);
	int sendPacket(const int nSocketFD, const char *szData);

private:
	std::map<int, int> mapClient;

};
