/*
 * CCmpClient.h
 *
 *  Created on: May 4, 2017
 *      Author: joe
 */

#ifndef GLOBAL_PROTOCOLHANDLER_CCMPCLIENT_H_
#define GLOBAL_PROTOCOLHANDLER_CCMPCLIENT_H_
#include <map>
#include "CATcpClient.h"
class CCmpClient: public CATcpClient
{

	typedef struct _CONF_CMP_CLIENT
	{
		bool bUseQueueReceive;
		void init()
		{
			bUseQueueReceive = false;
		}
	} CONF_CMP_CLIENT;

public:
	CCmpClient();
	virtual ~CCmpClient();
	int request(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData);
	int response(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData);
	void idleTimeout(bool bRun, int nIdleTime);
	void setUseQueueReceive(bool bEnable);

protected:
	void onTimer(int nId);
	void onReceive(unsigned long int nSocketFD, int nDataLen, const void* pData);
	int onTcpReceive(unsigned long int nSocketFD);

protected:
	virtual int onAccesslog(int nSocket, int nCommand, int nSequence, const void *szBody)
	{
		return 0;
	}
	;

private:
	typedef int (CCmpClient::*MemFn)(int, int, int, const void *);

	int sendPacket(int nSocket, int nCommand, int nStatus, int nSequence, const char *szData);
	CONF_CMP_CLIENT *confCmpClient;
	std::map<int, MemFn> mapFunc;

};

#endif /* GLOBAL_PROTOCOLHANDLER_CCMPCLIENT_H_ */
