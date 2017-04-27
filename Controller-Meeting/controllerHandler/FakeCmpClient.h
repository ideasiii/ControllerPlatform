#pragma once

#include "packet.h"

/**
* 與「偽﹒CMP」(Fake CMP) server 溝通的客戶端
* Fake CMP 與一般的 CMP 差異：
*   - client 不先送 bind request, server 也不理 bind request
*   - client 一連上 server 就直接送要做的命令
*   - server 在處理完第一通 request 就主動斷線
*/
class FakeCmpClient
{
public:
	FakeCmpClient(char *ip, int port);
	~FakeCmpClient();

	/**
	* 發送一個 CMP PDU 到 server 並接收回應，然後斷線
	* @param  requestPdu  傳送的 body 長度不可超過 MAX_DATA_LEN, header must be converted to network byte order
	* @param  responsePdu 預計收到的 body 長度不可超過 MAX_DATA_LEN, header is converted to host byte order
	* @return             how many bytes received (PDU length), <0 on error. Header in responsePdu will be converted
	*                     to host byte order on success.
	*/
	int sendOnlyOneRequest(CMP_PACKET *requestPdu, int reqPduLen, CMP_PACKET *responsePdu, char **errorDescription);
	
	/**
	* helper function for building request PDU
	* @param dst        output of pdu
	* @param bodyLength length of szData
	* @param szData     data size must not exceed MAX_DATA_LEN - sizeof(CMP_HEADER)
	* @return           if crafting process is ok, return the size of PDU, otherwise -1
	*/
	static int craftCmpPdu(CMP_PACKET *dst, int bodyLength, const int nCommandId,
		const int nStatus, const int nSequence, const uint8_t *szData);

private:
	char *serverIp;
	int serverPort;
};
