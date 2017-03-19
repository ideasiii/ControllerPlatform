/*
 * CMessageHandler.h
 *
 *  Created on: 2014年12月2日
 *      Author: jugo
 */

#pragma once

#define DATA_LEN     4096

/*
 * Declare the message structure.
 */
struct MESSAGE_BUF
{
		long mtype;                   // message filter, run use
		int nCommand;
		unsigned long int nId;
		int nDataLen;
		char cData[DATA_LEN];
};

class CMessageHandler
{
	public:
		CMessageHandler();
		virtual ~CMessageHandler();
		void close();
		int init(const long lkey);
		int sendMessage(int nType, int nCommand, unsigned long int nId, int nDataLen, const void* pData);
		int recvMessage(void **pbuf);
		void setRecvEvent(int nEvent);
		int getRecvEvent() const;
		int getMsqid() const;
		void setMsqid(int nId);
		int getBufLength() const;
		static int registerMsq(const long lkey);
		static void closeMsg(const int nMsqId);

	private:
		int msqid;
		MESSAGE_BUF message_buf;
		int buf_length;
		int m_nEvent;
};

