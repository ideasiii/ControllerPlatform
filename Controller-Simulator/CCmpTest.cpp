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
#ifdef AMX
	ss->runSocketReceive(nFD);
#else
	ss->runSMSSocketReceive(nFD);
#endif
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
				ntohl(pHeader->command_length), "", m_nSocketFD);
	}
	else
	{
		printf("Send CMP Request Fail!!\n");
	}

	return nRet;
}

int CCmpTest::sendRequestAMX(const int nCommandId)
{
	int nRet = -1;

	if (-1 == m_nSocketFD)
	{
		_DBG("TCP Socket invalid");
		return nRet;
	}

	string strCommand;
	int nPacketLen = 0;
	char buf[MAX_DATA_LEN];
	void *pbuf;
	pbuf = buf;

	switch (nCommandId)
	{
	case AMX_BIND:
		strCommand = "bind";
		break;
	case AMX_SYSTEM_ON:
		strCommand = "CTL_SYSTEM_ON";
		break;
	}

	char szEnd = 0x0D;
	printf("AMX command = %s\n", strCommand.c_str());
	memcpy(pbuf, strCommand.c_str(), strCommand.length());
	pbuf += strCommand.length();
	memcpy(pbuf, &szEnd, 1);
	nPacketLen += strCommand.length();
	++nPacketLen;

	nRet = send(m_nSocketFD, buf, nPacketLen, 0);

	printf("socket send: %s\n", buf);
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
	string strAppId = "123456789";
	string strMAC = "000c29d0013c";
//string strLogin = "{\"account\": \"akado\",	\"password\": \"oxymoron\",\"id\": \"000c29d0013c\",\"device\":0}";
	string strLogin = "{\"account\": \"akado\",	\"password\": \"akado\",\"id\": \"" + strMAC
			+ "\",\"device\":0,\"gcmid\":\"xxxxxxxxxxxxoooooooooooooo############\",\"model\":\"MH2LTU84P\"}";
	string strLogout = "{\"id\":\"" + strMAC + "\"}";
	string strSemantic = "{\"type\":0\"local\":0\"text\":\"Ivy Hello\"}";
	string strAMXControl = "{\"function\":1,\"device\":0,\"control\":1}";
	string strAMXStatus = "{\"function\":1,\"device\":0,\"request-status\":1}";

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
	case rdm_login_request:
		memcpy(pIndex, strLogin.c_str(), strLogin.size());
		pIndex += strLogin.size();
		nBody_len += strLogin.size();
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		break;
	case rdm_operate_request:
	case rdm_logout_request:
		memcpy(pIndex, strLogout.c_str(), strLogout.size());
		pIndex += strLogout.size();
		nBody_len += strLogout.size();
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
	case semantic_request:
		memcpy(pIndex, strSemantic.c_str(), strSemantic.size());
		pIndex += strSemantic.size();
		nBody_len += strSemantic.size();
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		break;
	case amx_control_request:
		memcpy(pIndex, strAMXControl.c_str(), strAMXControl.size());
		pIndex += strAMXControl.size();
		nBody_len += strAMXControl.size();
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
		break;
	case amx_status_request:
		memcpy(pIndex, strAMXStatus.c_str(), strAMXStatus.size());
		pIndex += strAMXStatus.size();
		nBody_len += strAMXStatus.size();
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
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
					ntohl(pHeader->command_length), "", m_nSocketFD);
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
					ntohl(pHeader->command_length), "", m_nSocketFD);
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
			printPacket(nCommand, nStatus, nSequence, nTotalLen, "", nSocketFD);
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

			printPacket(nCommand, nStatus, nSequence, nTotalLen, "", nSocketFD);

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

void CCmpTest::runSocketReceive(int nSocketFD)
{
	int nFD;
	int result;
	char pBuf[BUF_SIZE];
	string strPacket;

	while (1)
	{
		memset(pBuf, 0, sizeof(pBuf));
		result = recv(nSocketFD, pBuf, BUF_SIZE, MSG_NOSIGNAL);

		if (0 >= result)
		{
			close(nSocketFD);
			break;
		}

		strPacket = pBuf;
		printf("[Socket Client] socket receive : %s\n", strPacket.c_str());
		if (0 != strPacket.substr(0, 6).compare("CTL_OK") && 0 != strPacket.substr(0, 9).compare("CTL_ERROR"))
		{
			memset(pBuf, 0, sizeof(pBuf));
			memcpy(pBuf, "CTL_OK", 6);
			send(nSocketFD, pBuf, 6, MSG_NOSIGNAL);
			printf("[Socket Client] socket send : %s\n", pBuf);
		}
	}

	threadHandler->threadSleep(1);
	threadHandler->threadExit();
}
