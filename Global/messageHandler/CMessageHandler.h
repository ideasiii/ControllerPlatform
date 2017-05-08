/*
 * CMessageHandler.h
 *
 *  Created on: 2014年12月2日
 *      Author: jugo
 */

#pragma once

#include <string>

#define DATA_LEN     4096

typedef struct _Message
{
	int what;
	int arg1;
	int arg2;
	int opt;
	void *obj;
	char szData[2048];
	std::string strData;
} Message;

/*
 * Declare the message structure.
 */
struct MESSAGE_BUF
{
	long lFilter; // Message Filter, Who call CObject::run(long lFilter, const char * szDescript) will callback onReceiveMessage
	int nCommand;
	unsigned long int nId;
	int nDataLen;
	char cData[DATA_LEN];
	Message message;
};

class CMessageHandler
{
public:
	CMessageHandler();
	virtual ~CMessageHandler();
	void close();
	int init(const long lkey);
	int sendMessage(long lFilter, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
	int sendMessage(long lFilter, int nCommand, unsigned long int nId, int nDataLen, const void* pData,
			Message &message);
	int recvMessage(void **pbuf);
	void setRecvEvent(int nEvent);
	int getRecvEvent() const;
	int getMsqid() const;
	void setMsqid(int nId);
	int getBufLength() const;
	static int registerMsq(const long lkey);
	static void closeMsg(const int nMsqId);
	static void release();

private:
	int msqid;
	MESSAGE_BUF message_buf;
	int buf_length;
	int m_nEvent;

};

