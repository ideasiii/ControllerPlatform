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

class CCmpTest
{
public:
	CCmpTest();
	virtual ~CCmpTest();
	void cmpPressure();
	void ioPressure();
	int sendRequest(const int nCommandId);
	int sendRequestAMX(const int nCommandId);
	void connectController(const std::string strIP, const int nPort);
	void closeConnect();
	int getSocketfd() const;
	void runSMSSocketReceive(int nSocketFD);

private:
	int m_nSocketFD;
	int formatPacket(int nCommand, void **pPacket, int nSequence);
	std::string mstrToken;
	CThreadHandler *threadHandler;

};
