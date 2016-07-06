/*
 * CCmpTest.h
 *
 *  Created on: 2015年12月14日
 *      Author: Louis Ju
 */

#pragma once

#include <string>
class CThreadHandler;

class CCmpTest
{
public:
	CCmpTest();
	virtual ~CCmpTest();
	void cmpInitialRequest();
	void cmpSignupRequest();
	void cmpEnquireLinkRequest();
	void cmpAccessLogRequest();
	void cmpPressure();
	void ioPressure();
	void cmpMdmLogin();
	void cmpMdmOperate();
	void cmpPowerState();
	void cmpPowerSet();
	void cmpAuthentication();
	int sendRequest(const int nCommandId, void *pRespBuf);
	void connectController(const std::string strIP, const int nPort);
	void closeConnect();
	void cmpBind();
	int getSocketfd() const;
	void runSMSSocketReceive(int nSocketFD);

private:
	int m_nSocketFD;
	int formatPacket(int nCommand, void **pPacket, int nSequence);
	std::string mstrToken;
	CThreadHandler *threadHandler;

};
