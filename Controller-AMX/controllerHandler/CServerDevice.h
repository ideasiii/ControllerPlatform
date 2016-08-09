/*
 * CServerDevice.h
 *
 *  Created on: 2016年8月9日
 *      Author: root
 */

#pragma once

class CSocketServer;

class CServerDevice
{
public:
	static CServerDevice * getInstance();
	virtual ~CServerDevice();
	int startServer(const int nPort, const int nMsqId);
	void stopServer();

private:
	CServerDevice();
	CSocketServer * socketServer;
};
