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
#include "AMXCommand.h"

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
	ss->runCMPSocketReceive(nFD);
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

int CCmpTest::connectController(const std::string strIP, const int nPort)
{
	closeConnect();

	struct sockaddr_in hostAddr;
	if ((m_nSocketFD = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		_DBG("TCP Socket Create Fail!!\n");
		return FALSE;
	}

	hostAddr.sin_family = AF_INET;
	hostAddr.sin_addr.s_addr = inet_addr(strIP.c_str());
	hostAddr.sin_port = htons(nPort);
	if (connect(m_nSocketFD, (struct sockaddr *) &hostAddr, sizeof(struct sockaddr_in)) != 0)
	{
		_DBG("TCP Socket Connect Fail!!\n");
		return FALSE;
	}

	_DBG("TCP Socket connect success");

	threadHandler->createThread(threadSocketRecvHandler, this);

	return TRUE;
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

int CCmpTest::sendRequest(const int nCommandId, const char *szBody)
{
	int nRet = -1;
	int nPacketLen;

	if (-1 == m_nSocketFD)
	{
		_DBG("TCP Socket invalid");
		return nRet;
	}

	char buf[MAX_DATA_LEN];
	void *pbuf;
	pbuf = buf;

	nPacketLen = formatPacket(nCommandId, &pbuf, getSequence(), szBody);

	nRet = send(m_nSocketFD, pbuf, nPacketLen, 0);

	if (nPacketLen == nRet)
	{
		CMP_HEADER *pHeader;
		pHeader = (CMP_HEADER *) pbuf;

		printPacket(ntohl(pHeader->command_id), ntohl(pHeader->command_status), ntohl(pHeader->sequence_number),
				ntohl(pHeader->command_length), "", m_nSocketFD);
		printf("CMP Body: %s\n", szBody);
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
	char *pbuf;
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

int CCmpTest::formatPacket(int nCommand, void **pPacket, int nSequence, const char *szBody)
{
	static int snId = 0;
	char bufId[12];
	string strId;
	int nNum = 0;
	int net_type = 0;
	int nBody_len = 0;
	int nTotal_len;
	CMP_PACKET packet;
	char * pIndex;

	sprintf(bufId, "%d", ++snId);
	strId = bufId;

	if (27027 == nCommand)
		packet.cmpHeader.command_id = htonl(access_log_request);
	else
		packet.cmpHeader.command_id = htonl(nCommand);

	packet.cmpHeader.command_status = htonl( STATUS_ROK);
	packet.cmpHeader.sequence_number = htonl(nSequence);

	pIndex = packet.cmpBody.cmpdata;
	memset(packet.cmpBody.cmpdata, 0, sizeof(packet.cmpBody.cmpdata));

	string strControllerId = "123456789";
	//string strAccessLog =
	//	"\"PRODUCTION\":\"GSC大和^o^Y~~ي‎ al-ʻarabiyyahʻarabī \",\"PAGE\":\"我是測試檔123ABC ~@$我是测试档\",\"LOCATION\":\"25.0537591,121.5522948\",\"SOURCE_FROM\":\"justTest\",\"TYPE\":\"5\",\"ID\":\"AAAA1472030569161FFFF\",\"DATE\":\"2016-03-16 14:16:59\"}";
	string strAccessLog =
			"{\"TYPE\":\"2000\",\"ID\":\"dcd916573cc91456802938790gyvmac@gmail.com\",\"PAGE\":\"Application\",\"DATE\":\"2017-04-05  12:22:06\",\"LOCATION\":\"22.7896215,120.2836315\",\"SOURCE_FROM\":\"Soohoobook Inc.\"}";

	string strSignup =
			"{\"id\": \"" + strId
					+ "\",\"app_id\": \"987654321\",\"mac\": \"abcdefg\",\"os\": \"android\",\"phone\": \"0900000000\",\"fb_id\": \"fb1234\",\"fb_name\": \"louis\",\"fb_email\": \"louisju@iii.org.tw\",\"fb_account\": \"louisju@iii.org.tw\"}";
	string strAppId = "123456789";
	string strMAC = "000c29d0013c";
	string strBind = "{\"id\":\"000c29d0013c\"}";
	string strLogin =
			"{\"account\":\"akado\",\"password\":\"akado\",\"id\":\"000c29d0013c\",\"device\":0,\"gcmid\":\"xxxxxxxxxxxxoooooooooooooo############\",\"model\":\"MH2LTU84P\"}";
	string strLogout = "{\"id\":\"000c29d0013c\"}";
	string strSemantic = "{\"type\":0\"local\":0\"text\":\"Ivy Hello\"}";
	string strAMXControl = "{\"function\":4,\"device\":1,\"control\":2,\"TOKEN\":\"i_am_廢才\",\"ID\":\"you_are_廢才_too\"}";
	string strAMXStatus = "{\"function\":5,\"device\":5,\"request-status\":5}";
	string strAMXStatus2 = "{\"function\":4,\"device\":1,\"request-status\":2}";

	string strFCMIdRegister =
			"{\"FCM_ID\": \"fSk3qfqUxRk:APA91bHPz9WAsR5vxX9hNE-zoaKHwXVs2BJKW4V9guXnGWO1gSFh-EwjhJpZ00Y0DgOvGhVEg8u5iNDXC-ff6E14nuWzyuSq33H8SZPFbgUpIPiEggQYIbCnt1ZlZEjplZnU33akmPW+\",\"APP_ID\": \"1484537462214\",\"USER_ID\": \"d56e0b12-db99-11e6-bf26-cec0c932ce01\"}";
	string strFBToken = "";
	string strQRCodeTokn =
			"{\"QRCODE_TOKEN\": \"m+eJYbDinOt7XGXfVdBw5EZhQDDgmpnF8HXQr3Nkj6LBMSF+aGmoW//g54AXQcmrKl+gzOm4pz71tCLKOmR55g==\",\"USER_ID\": \"ffffffff-ffff-0000-0000-ffffffffffff\"}";
	string strSBAPPVersion = "";
	string strSBGetMeetingData = "{\"USER_ID\": \"ffffffff-ffff-0000-0000-ffffffffffff\"}";
	string strSBAmxControlAccess = "{\"USER_ID\": \"00000000-ffff-0000-ffff-ffffffffffff\",\"ROOM_ID\":\"ITES_101\"}";

	string strSBWirelessPowerCharge =
			"{\"APP_ID\": \"1484537462214\",\"USER_ID\": \"d56e0b12-db99-11e6-bf26-cec0c932ce01\",\"CHARGE_ PLACE\": \"ITES_FLOOR_1\"}";
	string strSematicWord = "{\"id\":0,\"type\":0,\"word\":\"我說一個故事給你們聽\",\"total\":0,\"number\":0}";
	string strDie = "{\"key\":\"suicide\"}";
	string strWheelPies = "";
	string strTTS =
			"{\"user_id\":\"\",\"voice_id\":0,\"emotion\":0,\"text\":\"我是三角形，出門去旅行。哎呦，哎呀，不小心跌了一跤。你看我。我看你。嗨！我們都是三角形！找到了朋友，好開心。哈哈哈，嘻嘻，一起在花園裡，飛來飛去。哈哈哈，嘻嘻，嗚，啦啦啦，啦啦啦，一起在池塘裡，游來游去，一起迎著風，轉來轉去。好涼喔，哈哈哈。\"}";

	if (0 != szBody)
	{
		memcpy(pIndex, szBody, strlen(szBody));
		pIndex += strlen(szBody);
		nBody_len += strlen(szBody);
		memcpy(pIndex, "\0", 1);
		pIndex += 1;
		nBody_len += 1;
	}
	else
	{
		switch (nCommand)
		{
		case bind_request:
			memcpy(pIndex, strBind.c_str(), strBind.size());
			pIndex += strBind.size();
			nBody_len += strBind.size();
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
		case 27027:
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
		case wheelpies_request:
			memcpy(pIndex, strSemantic.c_str(), strSemantic.size());
			pIndex += strSemantic.size();
			nBody_len += strSemantic.size();
			memcpy(pIndex, "\0", 1);
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

		case fcm_id_register_request:
			memcpy(pIndex, strFCMIdRegister.c_str(), strFCMIdRegister.size());
			pIndex += strFCMIdRegister.size();
			nBody_len += strFCMIdRegister.size();
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;
		case facebook_token_client_request:
			memcpy(pIndex, strFBToken.c_str(), strFBToken.size());
			pIndex += strFBToken.size();
			nBody_len += strFBToken.size();
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;
		case smart_building_qrcode_tokn_request:
			memcpy(pIndex, strQRCodeTokn.c_str(), strQRCodeTokn.size());
			pIndex += strQRCodeTokn.size();
			nBody_len += strQRCodeTokn.size();
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;
		case smart_building_appversion_request:

			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;
		case smart_building_getmeetingdata_request:
			memcpy(pIndex, strSBGetMeetingData.c_str(), strSBGetMeetingData.size());
			pIndex += strSBGetMeetingData.size();
			nBody_len += strSBGetMeetingData.size();
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;
		case smart_building_amx_control_access_request:
			memcpy(pIndex, strSBAmxControlAccess.c_str(), strSBAmxControlAccess.size());
			pIndex += strSBAmxControlAccess.size();
			nBody_len += strSBAmxControlAccess.size();
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;
		case smart_building_wireless_power_charge_request:
			memcpy(pIndex, strSBWirelessPowerCharge.c_str(), strSBWirelessPowerCharge.size());
			pIndex += strSBWirelessPowerCharge.size();
			nBody_len += strSBWirelessPowerCharge.size();
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;

		case 1166:
			memcpy(pIndex, strAMXStatus2.c_str(), strAMXStatus2.size());
			pIndex += strAMXStatus2.size();
			nBody_len += strAMXStatus2.size();
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;
		case semantic_word_request:
			memcpy(pIndex, strSematicWord.c_str(), strSematicWord.size());
			pIndex += strSematicWord.size();
			nBody_len += strSematicWord.size();
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;
		case controller_die_request:
			memcpy(pIndex, strDie.c_str(), strDie.size());
			pIndex += strDie.size();
			nBody_len += strDie.size();
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;
		case tts_request:
			memcpy(pIndex, strTTS.c_str(), strTTS.size());
			pIndex += strTTS.size();
			nBody_len += strTTS.size();
			memcpy(pIndex, "\0", 1);
			pIndex += 1;
			nBody_len += 1;
			break;
		}
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

	nPacketLen = formatPacket( semantic_word_request, &pbuf, getSequence());
	pHeader = (CMP_HEADER *) pbuf;

	while (1)
	{
		nRet = send(m_nSocketFD, pbuf, nPacketLen, 0);
		if (nPacketLen == nRet)
		{
			printPacket(ntohl(pHeader->command_id), ntohl(pHeader->command_status), ntohl(pHeader->sequence_number),
					ntohl(pHeader->command_length), "", m_nSocketFD);
		}
		else
		{
			printf("[Socket] send invalid, packet len: %d result len: %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",
					nPacketLen, nRet);
		}
		pHeader->sequence_number = htonl(getSequence());
		sleep(0.000001);
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
		sleep(0.000001);
	}
}

void CCmpTest::runCMPSocketReceive(int nSocketFD)
{
	_log("run Socket CMP Receive");

	int result = 0;
	char szTmp[16];
	int nTotalLen = 0;
	int nBodyLen = 0;
	int nCommand = generic_nack;
	int nSequence = 0;
	int nStatus = 0;

	CMP_PACKET cmpPacket;
	void* pHeader = &cmpPacket.cmpHeader;
	void* pBody;

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
				cmpHeader.command_status = htonl(nStatus);
				cmpHeader.sequence_number = htonl(nSequence);
				cmpHeader.command_length = htonl(sizeof(CMP_HEADER));
				send(nSocketFD, pHeaderResp, sizeof(CMP_HEADER), MSG_NOSIGNAL);
				continue;
			}

			if (enquire_link_request == (0x000000FF | nCommand))
				continue;

			nBodyLen = nTotalLen - sizeof(CMP_HEADER);
			char buffer[nBodyLen];
			if (0 < nBodyLen)
			{
				pBody = buffer;
				memset(buffer, 0, sizeof(buffer));
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

//	string console_cmd = "bye\r";
//	string tty = ttyname(STDIN_FILENO);
//	int fd = open(tty.c_str(), O_WRONLY);
//	write(fd, console_cmd.c_str(), console_cmd.size());
//	close(fd);
	printf("bye\n");
	exit(0);

	threadHandler->threadSleep(1);
	threadHandler->threadExit();
}

void CCmpTest::runSocketReceive(int nSocketFD)
{
	_log("run Socket Receive");

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
			int nSize = 0;
			string strCommand = trim(strPacket);
			if (AMX_STATUS_CURRENT.find(strCommand) != AMX_STATUS_CURRENT.end())
			{
				string strCurrent = AMX_STATUS_CURRENT[strCommand] + "\n";

				nSize = strCurrent.length();
				memcpy(pBuf, strCurrent.c_str(), nSize);
			}
			else
			{
				nSize = strlen("CTL_OK\n");
				memcpy(pBuf, "CTL_OK\n", nSize);
			}
			/*
			 if (0 == strPacket.substr(0, 13).compare("STATUS_SYSTEM"))
			 {
			 nSize = strlen("STATUS_SYSTEM_ON\n");
			 memcpy(pBuf, "STATUS_SYSTEM_ON\n", nSize);
			 }
			 else if (0 == strPacket.substr(0, 13).compare("STATUS_MATRIX"))
			 {
			 nSize = strlen("STATUS_MATRIX_INPUT3\n");
			 memcpy(pBuf, "STATUS_MATRIX_INPUT3\n", nSize);
			 }
			 else if (0 == strPacket.substr(0, strlen("STATUS_PROJ_POWER_LEFT")).compare("STATUS_PROJ_POWER_LEFT"))
			 {
			 nSize = strlen("STATUS_PROJ_ON_LEFT\n");
			 memcpy(pBuf, "STATUS_PROJ_ON_LEFT\n", nSize);
			 }

			 else
			 {
			 nSize = strlen("CTL_OK\n");
			 memcpy(pBuf, "CTL_OK\n", nSize);
			 }
			 */
			result = send(nSocketFD, pBuf, nSize, MSG_NOSIGNAL);
			if (0 >= result)
			{
				close(nSocketFD);
				break;
			}

			printf("[Socket Client] socket send size: %d , data: %s , ", result, pBuf);
		}
	}

	threadHandler->threadSleep(1);
	threadHandler->threadExit();
}
