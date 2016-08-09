/*
 * CServerAMX.h
 *
 *  Created on: 2016年8月9日
 *      Author: Jugo
 */

#pragma once

class CSocketServer;

class CServerAMX
{
public:
	static CServerAMX * getInstance();
	virtual ~CServerAMX();
	int startServer(const int nPort, const int nMsqId);
	void stopServer();

private:
	CServerAMX();
	CSocketServer * socketServer;
};
