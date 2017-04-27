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

#define REQUEST_PDU_SIZE_FLOOR 1
#define TCP_TIMEOUT 5
#define MAX_PDU_BODY_LEN MAX_DATA_LEN - (int)sizeof(CMP_HEADER)

FakeCmpClient::FakeCmpClient(char *ip, int port)
{
	this->serverIp = ip;
	this->serverPort = port;
}

FakeCmpClient::~FakeCmpClient()
{
}

int FakeCmpClient::sendOnlyOneRequest(CMP_PACKET *requestPdu, int reqPduLen, CMP_PACKET *responsePdu, char **errorDescription)
{
	int sockfd, ret, ioTryCount;
	struct sockaddr_in server;
	char respBuf[MAX_DATA_LEN];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		*errorDescription = "Could not create socket";
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
	printf("setsockopt 0 ret = %d\n", ret);
	if (ret == -1) {
		int error = errno;
		while ((close(sockfd) == -1) && (errno == EINTR));
		errno = error;
		return -1;
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	printf("setsockopt 1 ret = %d\n", ret);
	if (ret == -1) {
		int error = errno;
		while ((close(sockfd) == -1) && (errno == EINTR));
		errno = error;
		return -1;
	}

	// TODO print error on every failed call

	ret = connect(sockfd, (struct sockaddr *)&server, sizeof(server));
	printf("connect ret = %d\n", ret);
	if (ret < 0)
	{
		*errorDescription = strerror(errno);
		return -1;
	}

	int total = 0;
	ioTryCount = MAX_DATA_LEN;

	do
	{
		ret = send(sockfd, ((uint8_t*)requestPdu) + total, reqPduLen - total, 0);
		printf("send ret = %d\n", ret);
		if (ret < 0)
		{
			*errorDescription = "Send failed";
			close(sockfd);
			return -1;
		}

		total += ret;
		ioTryCount--;
	} while (total < reqPduLen && ioTryCount > 0);

	total = 0;
	do
	{
		ret = recv(sockfd, ((uint8_t*)responsePdu) + total, sizeof(CMP_PACKET) - total, 0);
		printf("recv ret = %d\n", ret);
		if (ret < 0)
		{
			*errorDescription = "Receive failed";
			close(sockfd);
			return -1;
		}

		total += ret;
		ioTryCount--;
	} while (total < (int)sizeof(CMP_HEADER) && ioTryCount > 0);

	CMP_HEADER *respHeader = ((CMP_HEADER*)responsePdu);
	if (respHeader->sequence_number != ((CMP_HEADER*)requestPdu)->sequence_number)
	{
		*errorDescription = "PDU sequence mismatch";
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
		*errorDescription = "Invalid PDU size in response header";
		close(sockfd);
		return -1;
	}

	ioTryCount = MAX_DATA_LEN;

	while (total < respHeader->command_length && ioTryCount > 0)
	{
		ret = recv(sockfd, ((uint8_t*)responsePdu) + total, respHeader->command_length - total, 0);
		if (ret < 0)
		{
			*errorDescription = "Receive failed";
			close(sockfd);
			return -1;
		}

		ioTryCount--;
		total += ret;
	}

	close(sockfd);

	if (total != respHeader->command_length)
	{
		*errorDescription = "Response size does not match declared";
		return -1;
	}

	return total;
}

int FakeCmpClient::craftCmpPdu(CMP_PACKET *dst, int bodyLength, const int nCommandId,
	const int nStatus, const int nSequence, const uint8_t *szData)
{
	if (dst == nullptr)
	{
		_log("[FakeCmpClient] dst is null");
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
			_log("[FakeCmpClient] holy crap pdu body is beyond %d (%d bytes here)", MAX_PDU_BODY_LEN, bodyLength);
			return -1;
		}

		memcpy(pBody, szData, bodyLength);
		//pBody[bodyLength] = '\0';
		//nTotalLen += bodyLength + 1;
		nTotalLen += bodyLength;

		if (nTotalLen < REQUEST_PDU_SIZE_FLOOR)
		{
			int originalLen = nTotalLen;
			nTotalLen = REQUEST_PDU_SIZE_FLOOR;

			int fd = open("/dev/urandom", O_RDONLY);
			read(fd, pBody + (originalLen - sizeof(CMP_PACKET)), nTotalLen - originalLen);
			close(fd);

			pBody[(REQUEST_PDU_SIZE_FLOOR - sizeof(CMP_HEADER)) - 1] = '\0';
		}
	}

	CMP_HEADER *header = &(dst->cmpHeader);
	header->command_id = htonl(nCommandId);
	header->command_status = htonl(nStatus);
	header->sequence_number = htonl(nSequence);
	header->command_length = htonl(nTotalLen);

	_log(("[FakeCmpClient] crafting request PDU, (commandLen, command, status, sequence) = (%d, %d, %d, %d)"),
		nTotalLen, nCommandId, nStatus, nSequence);

	return nTotalLen;
}
