/*
 * CCmpTest.h
 *
 *  Created on: 2015年12月14日
 *      Author: Louis Ju
 */

#pragma once

#include <string>
class CThreadHandler;

#define amx_bind_request	2001

enum
{
	AMX_BIND = 2001, AMX_SYSTEM_ON
};
class CCmpTest
{
public:
	CCmpTest();
	virtual ~CCmpTest();
	void cmpPressure();
	void ioPressure();
	int sendRequest(const int nCommandId, const char *szBody = 0);
	int sendRequestAMX(const int nCommandId);
	int connectController(const std::string strIP, const int nPort);
	void closeConnect();
	int getSocketfd() const;
	void runCMPSocketReceive(int nSocketFD);
	void runSocketReceive(int nSocketFD);

private:
	int m_nSocketFD;
	int formatPacket(int nCommand, void **pPacket, int nSequence, const char *szBody = 0);
	std::string mstrToken;
	CThreadHandler *threadHandler;

};
