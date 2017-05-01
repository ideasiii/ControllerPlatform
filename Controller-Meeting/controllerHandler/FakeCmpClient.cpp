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

#define TCP_TIMEOUT 2 // second

FakeCmpClient::FakeCmpClient(char *ip, int port)
	: serverIp(ip), serverPort(port)
{
}

FakeCmpClient::~FakeCmpClient()
{
}

void 

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

	struct timeval timeout;
	timeout.tv_sec = TCP_TIMEOUT;
	timeout.tv_usec = 0;

	socklen_t len = sizeof(timeout);
	ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
	_log("[FakeCmpClient] setsockopt() (SO_SNDTIMEO) ret = %d", ret);

	if (ret == -1) {
		_log("[FakeCmpClient] setsockopt() (SO_SNDTIMEO) failed (%d): %s", 
			errno, strerror(errno));
		close(sockfd);

		return -1;
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	_log("[FakeCmpClient] setsockopt() (SO_RCVTIMEO) ret (%d): %s", 
			errno, strerror(errno));
	
	if (ret == -1) {
		_log("[FakeCmpClient] setsockopt() (SO_RCVTIMEO) failed (%d): %s", 
			errno, strerror(errno));
		close(sockfd);
		
		return -1;
	}

	ret = connect(sockfd, (struct sockaddr *)&server, sizeof(server));
	_log("[FakeCmpClient] connect ret = %d", ret);

	if (ret < 0)
	{
		_log("[FakeCmpClient] connect() error (%d): %s", 
			errno, strerror(errno));
		close(sockfd);

		return -1;
	}

	int total = 0;
	ioTryCount = MAX_DATA_LEN;

	do
	{
		ret = send(sockfd, ((uint8_t*)requestPdu) + total, reqPduLen - total, 0);
		_log("[FakeCmpClient] send() ret = %d", ret);

		if (ret < 0)
		{
			_log("[FakeCmpClient] send() error (%d): %s", 
				errno, strerror(errno));
			close(sockfd);
			return -1;
		}

		total += ret;
		ioTryCount--;
		_log("[FakeCmpClient] send() total = %d", total);
	} while (total < reqPduLen && ioTryCount > 0);

	total = 0;
	do
	{
		ret = recv(sockfd, ((uint8_t*)responsePdu) + total, sizeof(CMP_PACKET) - total, 0);
		_log("[FakeCmpClient] recv() ret = %d", ret);

		if (ret == 0)
		{
			_log("[FakeCmpClient] recv() 0 server closed the socket while receiving");
			close(sockfd);
			return -1;
		}
		else if (ret < 0)
		{
			_log("[FakeCmpClient] recv() 0 error: (%d): %s", 
				errno, strerror(errno));
			close(sockfd);
			return -1;
		}

		total += ret;
		ioTryCount--;
		_log("[FakeCmpClient] recv() 0 total = %d", total);
	} while (total < (int)sizeof(CMP_HEADER) && ioTryCount > 0);

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

	ioTryCount = MAX_DATA_LEN;

	while (total < respHeader->command_length && ioTryCount > 0)
	{
		ret = recv(sockfd, ((uint8_t*)responsePdu) + total, respHeader->command_length - total, 0);
		
		if (ret == 0)
		{
			_log("[FakeCmpClient] recv() 1 server closed the socket while receiving");
			close(sockfd);
			return -1;
		}
		else if (ret < 0)
		{
			_log("[FakeCmpClient] recv() 1 error (%d): %s", 
				errno, strerror(errno));
			close(sockfd);
			return -1;
		}

		ioTryCount--;
		total += ret;
		_log("[FakeCmpClient] recv() 1 total = %d", total);
	}

	close(sockfd);

	if (total != respHeader->command_length)
	{
		_log("[FakeCmpClient] Response size does not match header (%d vs. %d)",
			, total, respHeader->command_length);
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
	CMP_HEADER *pHeader = &(dst->cmpHeader);

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
