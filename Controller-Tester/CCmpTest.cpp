/*
 * CCmpTest.cpp
 *
 *  Created on: 2015年12月14日
 *      Author: Louis Ju
 */

#include <stdio.h>               // for printf() and fprintf()
#include <sys/socket.h>     // for socket(), bind(), and connect()
#include <arpa/inet.h>       // for sockaddr_in and inet_ntoa()
#include <stdlib.h>              // for atoi() and exit()
#include <string.h>              // for memset()
#include <unistd.h>             // for close()
#include <fcntl.h>                // for fcntl()
#include <errno.h>
#include <sys/epoll.h>
#include <memory.h>
#include <ctime>
#include "CCmpTest.h"
#include "common.h"
#include "packet.h"
#include "utility.h"
#include "CThreadHandler.h"

#define TRACKER_MOBILE		1
#define TRACKER_CHARGIN		2
#define TRACKER_MORE_SDK	3
#define TRACKER_SERVICE		4
#define	TRACKER_APPLIANCE	5
#define TRACKER_TOY				6
#define TRACKER_IOT				7
#define TYPE_TEST						20160604

void *threadSocketRecvHandler(void *argv)
{
	int nFD;
	CCmpTest* ss = reinterpret_cast<CCmpTest*>(argv);
	nFD = ss->getSocketfd();
	ss->runSMSSocketReceive(nFD);
	return NULL;
}

CCmpTest::CCmpTest() :
		m_nSocketFD(-1), threadHandler(new CThreadHandler)
{

}

CCmpTest::~CCmpTest()
{
	if (-1 != m_nSocketFD)
	{
		close(m_nSocketFD);
		m_nSocketFD = -1;
	}
}

void CCmpTest::connectController(const std::string strIP, const int nPort)
{
	closeConnect();

	struct sockaddr_in hostAddr;
	if ((m_nSocketFD = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		_DBG("TCP Socket Create Fail!!\n");
		return;
	}

	hostAddr.sin_family = AF_INET;
	hostAddr.sin_addr.s_addr = inet_addr(strIP.c_str());
	hostAddr.sin_port = htons(nPort);
	if (connect(m_nSocketFD, (struct sockaddr *) &hostAddr, sizeof(struct sockaddr_in)) != 0)
	{
		_DBG("TCP Socket Connect Fail!!\n");
		return;
	}

	_DBG("TCP Socket connect success");

	threadHandler->createThread(threadSocketRecvHandler, this);
}

void CCmpTest::closeConnect()
{
	if (-1 != m_nSocketFD)
	{
		close(m_nSocketFD);
		m_nSocketFD = -1;
	}
}

int CCmpTest::getSocketfd() const
{
	return m_nSocketFD;
}

int CCmpTest::sendRequest(const int nCommandId)
{
	int nRet = -1;

	if (-1 == m_nSocketFD)
	{
		_DBG("TCP Socket invalid");
		return nRet;
	}

	char buf[MAX_DATA_LEN];
	void *pbuf;
	pbuf = buf;

	int nPacketLen = formatPacket(nCommandId, &pbuf, getSequence());
	nRet = send(m_nSocketFD, pbuf, nPacketLen, 0);
	if (nPacketLen == nRet)
	{
		CMP_HEADER *pHeader;
		pHeader = (CMP_HEADER *) pbuf;
		printPacket(ntohl(pHeader->command_id), ntohl(pHeader->command_status), ntohl(pHeader->sequence_number),
				ntohl(pHeader->command_length), m_nSocketFD);
	}
	else
	{
		printf("Send CMP Request Fail!!\n");
	}

	return nRet;
}

int CCmpTest::formatPacket(int nCommand, void **pPacket, int nSequence)
{
	int nNum = 0;
	int net_type = 0;
	int nBody_len = 0;
	int nTotal_len;
	CMP_PACKET packet;
	char * pIndex;

	packet.cmpHeader.command_id = htonl(nCommand);
	packet.cmpHeader.command_status = htonl( STATUS_ROK);
	packet.cmpHeader.sequence_number = htonl(nSequence);

	pIndex = packet.cmpBody.cmpdata;
	memset(packet.cmpBody.cmpdata, 0, sizeof(packet.cmpBody.cmpdata));

	string strControllerId = "123456789";
	string strAccessLog =
			"{\"PRODUCTION\":\"GSC大和^o^Y~~ي‎ al-ʻarabiyyahʻarabī \",\"PAGE\":\"我是測試檔123ABC ~@$我是测试档\",\"LOCATION\":\"25.0537591,121.5522948\",\"SOURCE_FROM\":\"justTest\",\"TYPE\":\"5\",\"ID\":\"1462241606197\",\"PRICE\":\"1500\",\"DATE\":\"2016-03-16 14:16:59\"}";
	string strSignup =
			"{\"id\": \"1234567890\",\"app_id\": \"987654321\",\"mac\": \"abcdefg\",\"os\": \"android\",\"phone\": \"0900000000\",\"fb_id\": \"fb1234\",\"fb_name\": \"louis\",\"fb_email\": \"louisju@iii.org.tw\",\"fb_account\": \"louisju@iii.org.tw\"}";
	string strMdmAccount = "testing@iii.org.tw";
	string strMdmPasswd = "testing";
	string strAppId = "123456789";
	string strMAC = "000c29d0013c";

	switch (nCommand)
	{
	case bind_request:
		memcpy(pIndex, strMAC.c_str(), strMAC.size());
		pIndex += strMAC.size();
		nBody_len += strMAC.size();
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		break;
	case access_log_request:
		net_type = htonl(TYPE_TEST);
		memcpy(pIndex, (const char*) &net_type, 4);
		pIndex += 4;
		nBody_len += 4;
		memcpy(pIndex, strAccessLog.c_str(), strAccessLog.length()); //	log data
		pIndex += strAccessLog.length();
		nBody_len += strAccessLog.length();
		memcpy(pIndex, "\0", 1);
		++pIndex;
		++nBody_len;
		break;
	case initial_request:
		net_type = htonl(TRACKER_APPLIANCE);
		memcpy(pIndex, (const char*) &net_type, 4);
		pIndex += 4;
		nBody_len += 4;
		break;
	case sign_up_request:
		net_type = htonl(TYPE_MOBILE_SERVICE);
		memcpy(pIndex, (const char*) &net_type, 4);
		pIndex += 4;
		nBody_len += 4;
		memcpy(pIndex, strSignup.c_str(), strSignup.length()); //	sign up data
		pIndex += strSignup.length();
		nBody_len += strSignup.length();
		memcpy(pIndex, "\0", 1);
		++pIndex;
		++nBody_len;
		break;
	case mdm_login_request:
		memcpy(pIndex, strMdmAccount.c_str(), strMdmAccount.size());
		pIndex += strMdmAccount.size();
		nBody_len += strMdmAccount.size();
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		memcpy(pIndex, strMdmPasswd.c_str(), strMdmPasswd.size());
		pIndex += strMdmPasswd.size();
		nBody_len += strMdmPasswd.size();
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		break;
	case mdm_operate_request:
		if (mstrToken.empty())
		{
			mstrToken = "123456789";
		}
		memcpy(pIndex, mstrToken.c_str(), mstrToken.size());
		pIndex += mstrToken.size();
		nBody_len += mstrToken.size();
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		break;
	case power_port_state_request:
		nNum = htonl(1);
		memcpy(pIndex, (const char*) &nNum, 4);
		pIndex += 4;
		nBody_len += 4;
		break;
	case power_port_set_request:
		nNum = htonl(1);
		memcpy(pIndex, (const char*) &nNum, 4);
		pIndex += 4;
		nBody_len += 4;

		nNum = htonl(1);
		memcpy(pIndex, (const char*) &nNum, 4);
		pIndex += 4;
		nBody_len += 4;

		memcpy(pIndex, "0", 1);
		pIndex += 1;
		nBody_len += 1;
		break;
	case authentication_request:
		break;
	}

	nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);
	memcpy(*pPacket, &packet, nTotal_len);

	return nTotal_len;

}

void CCmpTest::cmpPressure()
{
	int nPacketLen = 0;
	int nRet = 0;
	char buf[MAX_DATA_LEN];
	void *pbuf;
	pbuf = buf;
	CMP_HEADER *pHeader;

	nPacketLen = formatPacket( access_log_request, &pbuf, getSequence());
	pHeader = (CMP_HEADER *) pbuf;

	while (1)
	{
		nRet = send(m_nSocketFD, pbuf, nPacketLen, 0);
		if (nPacketLen == nRet)
		{
			printPacket(ntohl(pHeader->command_id), ntohl(pHeader->command_status), ntohl(pHeader->sequence_number),
					ntohl(pHeader->command_length), m_nSocketFD);
		}
		pHeader->sequence_number = htonl(getSequence());
	}
}

void CCmpTest::ioPressure()
{
	int nPacketLen = 0;
	int nRet = 0;
	char buf[MAX_DATA_LEN];
	void *pbuf;
	pbuf = buf;
	CMP_HEADER *pHeader;

	nPacketLen = formatPacket( enquire_link_request, &pbuf, getSequence());
	pHeader = (CMP_HEADER *) pbuf;

	while (1)
	{
		nRet = send(m_nSocketFD, pbuf, nPacketLen, 0);
		if (nPacketLen == nRet)
		{
			printPacket(ntohl(pHeader->command_id), ntohl(pHeader->command_status), ntohl(pHeader->sequence_number),
					ntohl(pHeader->command_length), m_nSocketFD);
		}
		pHeader->sequence_number = htonl(getSequence());
		//sleep(0.000001);
	}
}

void CCmpTest::runSMSSocketReceive(int nSocketFD)
{
	int result = 0;
	char szTmp[16];
	int nTotalLen = 0;
	int nBodyLen = 0;
	int nCommand = generic_nack;
	int nSequence = 0;
	int nStatus = 0;

	CMP_PACKET cmpPacket;
	void* pHeader = &cmpPacket.cmpHeader;
	void* pBody = &cmpPacket.cmpBody;

	CMP_HEADER cmpHeader;
	void *pHeaderResp = &cmpHeader;
	int nCommandResp;

	while (1)
	{
		memset(&cmpPacket, 0, sizeof(CMP_PACKET));
		result = recv(nSocketFD, pHeader, sizeof(CMP_HEADER), MSG_NOSIGNAL);

		if (sizeof(CMP_HEADER) == result)
		{
			nCommand = ntohl(cmpPacket.cmpHeader.command_id);
			nStatus = ntohl(cmpPacket.cmpHeader.command_status);
			nSequence = ntohl(cmpPacket.cmpHeader.sequence_number);
			nTotalLen = ntohl(cmpPacket.cmpHeader.command_length);
			if ( enquire_link_request == nCommand)
			{
				memset(&cmpHeader, 0, sizeof(CMP_HEADER));
				nCommandResp = generic_nack | nCommand;
				cmpHeader.command_id = htonl(nCommandResp);
				cmpHeader.command_status = htonl( STATUS_ROK);
				cmpHeader.sequence_number = htonl(nSequence);
				cmpHeader.command_length = htonl(sizeof(CMP_HEADER));
				send(nSocketFD, pHeaderResp, sizeof(CMP_HEADER), MSG_NOSIGNAL);
				continue;
			}

			if (enquire_link_response == nCommand)
				continue;

			printPacket(nCommand, nStatus, nSequence, nTotalLen, nSocketFD);

			nBodyLen = nTotalLen - sizeof(CMP_HEADER);

			if (0 < nBodyLen)
			{
				result = recv(nSocketFD, pBody, nBodyLen, MSG_NOSIGNAL);
				if (result != nBodyLen)
				{
					close(nSocketFD);
					printf("[Socket Client] socket client close : %d , packet length error: %d != %d\n", nSocketFD,
							nBodyLen, result);
					break;
				}
				printf("[Socket Client] socket receive CMP Body: %s\n", static_cast<char*>(pBody));
			}
		}
		else
		{

			close(nSocketFD);
			printf("[Socket Client] socket client close : %d , packet header length error: %d\n", nSocketFD, result);
			break;
		}

		if (0 >= result)
		{
			close(nSocketFD);
			break;
		}

	} // while

	threadHandler->threadSleep(1);
	threadHandler->threadExit();
}
