#include "FakeCmpClient.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sstream>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PDU_BODY_LEN (int)(MAX_DATA_LEN - sizeof(CMP_HEADER))

#define TCP_TIMEOUT 3 // second

int setSocketTimeout(int sockfd);
int sendToSocket(int sockfd, const uint8_t *buf, int len);
int recvFromSocket(int sockfd, uint8_t *buf, int len, int bufLen);

FakeCmpClient::FakeCmpClient(char *ip, int port)
	: serverIp(ip), serverPort(port)
{
}

FakeCmpClient::~FakeCmpClient()
{
}

int FakeCmpClient::sendOnlyOneRequest(CMP_PACKET *requestPdu, int reqPduLen, CMP_PACKET *responsePdu)
{
	int sockfd, ret, ioTryCount;
	struct sockaddr_in server;
	char respBuf[MAX_DATA_LEN];

	_log("[FakeCmpClient] Enter sendOnlyOneRequest()");

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		_log("[FakeCmpClient] Could not create socket");
		return -1;
	}

	server.sin_addr.s_addr = inet_addr(this->serverIp);
	server.sin_family = AF_INET;
	server.sin_port = htons(this->serverPort);

	ret = setSocketTimeout(sockfd);
	if (ret == -1)
	{
		close(sockfd);
		return -1;
	}

	ret = connect(sockfd, (struct sockaddr *)&server, sizeof(server));
	_log("[FakeCmpClient] connect ret() = %d", ret);

	if (ret < 0)
	{
		_log("[FakeCmpClient] connect() error (%d): %s", 
			errno, strerror(errno));
		close(sockfd);

		return -1;
	}
		
	ret = sendToSocket(sockfd, (uint8_t*)requestPdu, reqPduLen);
	if (ret != reqPduLen)
	{
		_log("[FakeCmpClient] sendToSocket() ret != reqPduLen (%d != %d)", ret, reqPduLen);
		close(sockfd);

		return -1;
	}

	int total = 0;
	ret = recvFromSocket(sockfd, ((uint8_t*)responsePdu), sizeof(CMP_HEADER), sizeof(CMP_PACKET));
	total += ret;

	if (ret < (int)sizeof(CMP_HEADER))
	{
		_log("[FakeCmpClient] recvFromSocket() ret is less than sizeof(CMP_HEADER)");
		close(sockfd);

		return -1;
	}

	CMP_HEADER *respHeader = ((CMP_HEADER*)responsePdu);
	if (respHeader->sequence_number != ((CMP_HEADER*)requestPdu)->sequence_number)
	{
		_log("[FakeCmpClient] PDU sequence mismatch (%d vs. %d)", 
			ntohl(respHeader->sequence_number), ntohl(((CMP_HEADER*)requestPdu)->sequence_number));
		close(sockfd);
		
		return -1;
	}

	respHeader->command_id = ntohl(respHeader->command_id);
	respHeader->command_status = ntohl(respHeader->command_status);
	respHeader->sequence_number = ntohl(respHeader->sequence_number);
	respHeader->command_length = ntohl(respHeader->command_length);

	if (respHeader->command_length < (int)sizeof(CMP_HEADER)
		|| respHeader->command_length > MAX_DATA_LEN)
	{
		_log("[FakeCmpClient] Invalid command length (%d) in response header", 
			respHeader->command_length);
		close(sockfd);
		
		return -1;
	}

	ret = recvFromSocket(sockfd, ((uint8_t*)responsePdu) + total,
		respHeader->command_length - total, (int)sizeof(CMP_PACKET) - total);
	
	total += ret;
	close(sockfd);

	if (total != respHeader->command_length)
	{
		_log("[FakeCmpClient] Response size does not match header (%d vs. %d)",
			total, respHeader->command_length);
		return -1;
	}

	_log("[FakeCmpClient] Send & Recv ok, recv size = %d", total);

	return total;
}

int FakeCmpClient::craftCmpPdu(CMP_PACKET *dst, int bodyLength, const int nCommandId,
	const int nStatus, const int nSequence, const uint8_t *szData)
{
	if (dst == nullptr)
	{
		_log("[FakeCmpClient] craftCmpPdu: dst is null");
		return -1;
	}

	int nTotalLen = sizeof(CMP_HEADER);
	memset(dst, 0, sizeof(CMP_PACKET));

	if (szData != nullptr)
	{
		char *pBody = dst->cmpBody.cmpdata;

		if (bodyLength > MAX_PDU_BODY_LEN)
		{
			_log("[FakeCmpClient] craftCmpPdu: PDU body exceeds %d (%d bytes here)", 
				MAX_PDU_BODY_LEN, bodyLength);
			return -1;
		}

		memcpy(pBody, szData, bodyLength);
		nTotalLen += bodyLength;
	}

	CMP_HEADER *header = &(dst->cmpHeader);
	header->command_id = htonl(nCommandId);
	header->command_status = htonl(nStatus);
	header->sequence_number = htonl(nSequence);
	header->command_length = htonl(nTotalLen);

	_log("[FakeCmpClient] craftCmpPdu: craft request PDU ok, "
		"(len, command, status, sequence) = (%d, %d, %d, %d)",
		nTotalLen, nCommandId, nStatus, nSequence);

	return nTotalLen;
}

int setSocketTimeout(int sockfd)
{
	struct timeval timeout;
	timeout.tv_sec = TCP_TIMEOUT;
	timeout.tv_usec = 0;

	int ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
	_log("[FakeCmpClient] setsockopt() (SO_SNDTIMEO) ret = %d", ret);

	if (ret == -1) {
		_log("[FakeCmpClient] setsockopt() (SO_SNDTIMEO) failed (%d): %s",
			errno, strerror(errno));

		return -1;
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	_log("[FakeCmpClient] setsockopt() (SO_RCVTIMEO) ret = %d", ret);

	if (ret == -1) {
		_log("[FakeCmpClient] setsockopt() (SO_RCVTIMEO) failed (%d): %s",
			errno, strerror(errno));

		return -1;
	}

	return 0;
}

int sendToSocket(int sockfd, const uint8_t *buf, int len)
{
	int total = 0, ioTryCount = MAX_DATA_LEN, ret;

	do
	{
		ret = send(sockfd, buf + total, len - total, 0);
		_log("[FakeCmpClient] send() ret = %d", ret);

		if (ret < 0)
		{
			_log("[FakeCmpClient] send() error (%d): %s",
				errno, strerror(errno));
			
			return -1;
		}

		total += ret;
		ioTryCount -= 1;
		_log("[FakeCmpClient] send() total = %d", total);
	} while (total < len && ioTryCount > 0);

	return total;
}

int recvFromSocket(int sockfd, uint8_t *buf, int len, int bufLen)
{
	int total = 0, ret;

	while (total < len)
	{
		ret = recv(sockfd, buf + total, bufLen - total, 0);
		_log("[FakeCmpClient] recv() ret = %d", ret);

		if (ret == 0)
		{
			_log("[FakeCmpClient] recv() server closed the socket while receiving");
			return -1;
		}
		else if (ret < 0)
		{
			_log("[FakeCmpClient] recv() error: (%d): %s",
				errno, strerror(errno));
			return -1;
		}

		total += ret;
		_log("[FakeCmpClient] recv() total = %d", total);
	}

	return total;
}
