/*
 * CEvilTest.cpp
 *
 *  Created on: 2017年3月21日
 *      Author: Jugo
 */
#include <sys/socket.h>     // for socket(), bind(), and connect()
#include <arpa/inet.h>       // for sockaddr_in and inet_ntoa()
#include <stdio.h>               // for printf() and fprintf()
#include <string.h>              // for memset()
#include <unistd.h>             // for close()

#include "CEvilTest.h"
#include "CThreadHandler.h"
#include "packet.h"

void *threadEvilClient(void *argv)
{
	CEvilTest* ss = reinterpret_cast<CEvilTest*>(argv);
	ss->run();
	return 0;
}

CEvilTest::CEvilTest(const char *szIP, int nPort) :
		mnPort(0), thread(new CThreadHandler())
{
	strcpy(mszIP, szIP);
	mnPort = nPort;
}

CEvilTest::~CEvilTest()
{

}

void CEvilTest::start(int nCount)
{
	for(int i = 0; i < nCount; ++i)
	{
		thread->createThread(threadEvilClient, this);
	}
}

static int snCount = 0;

void CEvilTest::run()
{
	char bufId[12];
	string strId;
	int nSocketFD;
	int nSeq = ++snCount;
	sprintf(bufId, "%d", nSeq);
	strId = bufId;

	if(0x7FFFFFFF <= snCount)
		snCount = 0;

	printf("Connect: %s : %d %d\n", mszIP, mnPort, nSeq);

	struct sockaddr_in hostAddr;
	if((nSocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		printf("TCP Socket Create Fail!!\n");
		thread->threadExit();
		return;
	}

	hostAddr.sin_family = AF_INET;
	hostAddr.sin_addr.s_addr = inet_addr(mszIP);
	hostAddr.sin_port = htons(mnPort);
	if(connect(nSocketFD, (struct sockaddr *) &hostAddr, sizeof(struct sockaddr_in)) != 0)
	{
		printf("TCP Socket Connect Fail!!\n");
		thread->threadExit();
		return;
	}

	printf("TCP Socket connect success\n");

	char buf[2048];
	void *pbuf;
	pbuf = buf;
	int nBody_len = 0;
	CMP_PACKET packet;
	CMP_HEADER* pHeader = &packet.cmpHeader;
	void* pBody = &packet.cmpBody;
	char * pIndex;

	packet.cmpHeader.command_id = htonl(sign_up_request);
	packet.cmpHeader.command_status = htonl(STATUS_ROK);
	packet.cmpHeader.sequence_number = htonl(nSeq);

	pIndex = packet.cmpBody.cmpdata;
	memset(packet.cmpBody.cmpdata, 0, sizeof(packet.cmpBody.cmpdata));

	string strSignup =
			"{\"id\": \"" + strId
					+ "\",\"app_id\": \"987654321\",\"mac\": \"abcdefg\",\"os\": \"android\",\"phone\": \"0900000000\",\"fb_id\": \"fb1234\",\"fb_name\": \"louis\",\"fb_email\": \"louisju@iii.org.tw\",\"fb_account\": \"louisju@iii.org.tw\"}";

	int net_type = htonl(TYPE_MOBILE_SERVICE);
	memcpy(pIndex, (const char*) &net_type, 4);
	pIndex += 4;
	nBody_len += 4;
	memcpy(pIndex, strSignup.c_str(), strSignup.length()); //	sign up data
	pIndex += strSignup.length();
	nBody_len += strSignup.length();
	memcpy(pIndex, "\0", 1);
	++pIndex;
	++nBody_len;

	int nTotal_len = sizeof(CMP_HEADER) + nBody_len;
	packet.cmpHeader.command_length = htonl(nTotal_len);
	memcpy(pbuf, &packet, nTotal_len);

	int nRet = send(nSocketFD, pbuf, nTotal_len, 0);

	if(nRet == nTotal_len)
	{
		pHeader = (CMP_HEADER *) pbuf;

		printPacket(ntohl(pHeader->command_id), ntohl(pHeader->command_status), ntohl(pHeader->sequence_number),
				ntohl(pHeader->command_length), "", nSocketFD);
	}
	else
	{
		printf("send package fail ###################################\n");
	}

	memset(&packet, 0, sizeof(CMP_PACKET));
	memset(pHeader, 0, sizeof(CMP_HEADER));
	nRet = recv(nSocketFD, pHeader, sizeof(CMP_HEADER), MSG_NOSIGNAL);

	if(sizeof(CMP_HEADER) == nRet)
	{
		int nCommand = ntohl(packet.cmpHeader.command_id);
		int nStatus = ntohl(packet.cmpHeader.command_status);
		int nSequence = ntohl(packet.cmpHeader.sequence_number);
		int nTotalLen = ntohl(packet.cmpHeader.command_length);
		printPacket(nCommand, nStatus, nSequence, nTotalLen, "", nSocketFD);
		int nBodyLen = nTotalLen - sizeof(CMP_HEADER);

		if(0 < nBodyLen)
		{
			nRet = recv(nSocketFD, pBody, nBodyLen, MSG_NOSIGNAL);
			if(nRet == nBodyLen)
			{

				printf("[Socket Client] socket receive CMP Body: %s\n", static_cast<char*>(pBody));
			}

		}
	}

	close(nSocketFD);

	thread->threadExit();
}

